#include <QtTest>
#include <QEventLoop>
#include <QFile>
#include <QImage>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTcpServer>
#include <QTemporaryDir>
#include <QTimer>
#include <stdexcept>
#include "Infrastructure/Http/FakeServer.h"

namespace {
struct Response {
    int status;
    QByteArray body;
    QByteArray retry_after;
    QByteArray cache_control;
};

Response request(quint16 port, const QString& path, const QByteArray& body = {}, bool post = false,
                 const QByteArray& forwarded_for = {}) {
    QNetworkAccessManager network;
    QNetworkRequest request(QUrl(QString("http://127.0.0.1:%1%2").arg(port).arg(path)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (!forwarded_for.isEmpty()) {
        request.setRawHeader("X-Forwarded-For", forwarded_for);
        request.setRawHeader("Forwarded", "for=" + forwarded_for);
    }
    auto* reply = post ? network.post(request, body) : network.get(request);
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    timeout.start(3000);
    loop.exec();
    return {reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), reply->readAll(),
            reply->rawHeader("Retry-After"), reply->rawHeader("Cache-Control")};
}

const QByteArray credentials = R"({"email":"test@example.com","password":"password"})";
} // namespace

class BackendServerTests : public QObject {
    Q_OBJECT
  private slots:
    void initTestCase() {
        // Prevent accidental live database access, including the login failure check.
        original_port_ = qgetenv("PORT");
        had_port_ = qEnvironmentVariableIsSet("PORT");
        original_uri_ = qgetenv("MONGODB_URI");
        had_uri_ = qEnvironmentVariableIsSet("MONGODB_URI");
        qputenv("MONGODB_URI", "invalid-uri-that-must-not-be-used");
    }

    void init() {
        // The current server starts in its constructor and reads PORT.
        QTcpServer reservation;
        QVERIFY(reservation.listen(QHostAddress::LocalHost, 0));
        port_ = reservation.serverPort();
        qputenv("PORT", QByteArray::number(port_));
    }

    void cleanupTestCase() {
        if (had_port_)
            qputenv("PORT", original_port_);
        else
            qunsetenv("PORT");
        if (had_uri_)
            qputenv("MONGODB_URI", original_uri_);
        else
            qunsetenv("MONGODB_URI");
    }

    void starts_on_configured_port_and_serves_requests() {
        MongoDBDataManager database;
        Server server(database, nullptr);
        QVERIFY(server.is_listening());
        const auto response = request(port_, "/");
        QCOMPARE(response.status, 200);
        QCOMPARE(response.body, QByteArray(INVOKE_APP_VERSION));
    }

    void occupied_port_reports_failure_and_new_server_can_start() {
        QTcpServer occupied;
        QVERIFY(occupied.listen(QHostAddress::AnyIPv4, port_));
        MongoDBDataManager database;
        {
            Server server(database, nullptr);
            QVERIFY(!server.is_listening());
        }
        occupied.close();
        Server retry(database, nullptr);
        QVERIFY(retry.is_listening());
        QCOMPARE(request(port_, "/").status, 200);
    }

    void unauthenticated_invoice_and_data_routes_are_protected() {
        MongoDBDataManager database;
        Server server(database, nullptr);
        QVERIFY(server.is_listening());
        QCOMPARE(request(port_, "/invoices/stock-list", "[]", true).status, 401);
        QCOMPARE(request(port_, "/invoices/invoice_start", "{}", true).status, 401);
        QCOMPARE(request(port_, "/business/list").status, 401);
        QCOMPARE(request(port_, "/stock/list").status, 401);
        QCOMPARE(request(port_, "/data/business", "{}", true).status, 401);
        QCOMPARE(request(port_, "/auth/logout", "{}", true).status, 200);
        QCOMPARE(request(port_, "/auth/logout", "{}", true).status, 200);
    }

    void login_rejects_bad_requests_and_reports_database_unavailability() {
        MongoDBDataManager database;
        Server server(database, nullptr);
        QVERIFY(server.is_listening());
        QCOMPARE(request(port_, "/auth/login", "not-json", true).status, 400);
        // An invalid URI ensures this exercises the failure path without a live DB.
        QCOMPARE(request(port_, "/auth/login", credentials, true).status, 503);
        QCOMPARE(request(port_, "/").status, 200);
    }

    void login_account_quota_normalizes_email_and_recovers_at_expiry() {
        using namespace Infrastructure::Security;
        auto now = LocalRateLimiter::Clock::time_point{};
        auto limiter = std::make_unique<LocalRateLimiter>([&] { return now; });
        MongoDBDataManager database;
        Server server(database, nullptr, std::move(limiter));
        QVERIFY(server.is_listening());
        for (int i = 0; i < 5; ++i)
            QCOMPARE(request(port_, "/auth/login", credentials, true).status, 503);
        now += std::chrono::milliseconds(59001);
        const auto blocked = request(port_, "/auth/login",
            R"({"email":" TEST@EXAMPLE.COM ","password":"different"})", true);
        QCOMPARE(blocked.status, 429);
        QCOMPARE(blocked.retry_after, QByteArray("1"));
        QCOMPARE(blocked.cache_control, QByteArray("no-store"));
        // A different account still reaches authentication (the invalid DB URI).
        QCOMPARE(request(port_, "/auth/login",
            R"({"email":"other@example.com","password":"password"})", true).status, 503);
        now += std::chrono::milliseconds(999);
        QCOMPARE(request(port_, "/auth/login", credentials, true).status, 503);
    }

    void login_peer_quota_covers_malformed_requests_and_ignores_forwarded_headers() {
        MongoDBDataManager database;
        Server server(database, nullptr);
        QVERIFY(server.is_listening());
        for (int i = 0; i < 20; ++i)
            QCOMPARE(request(port_, "/auth/login", "not-json", true,
                "192.0.2." + QByteArray::number(i + 1)).status, 400);
        const auto blocked = request(port_, "/auth/login", credentials, true, "198.51.100.1");
        QCOMPARE(blocked.status, 429);
        QVERIFY(blocked.retry_after.toInt() >= 1 && blocked.retry_after.toInt() <= 60);
        QCOMPARE(blocked.cache_control, QByteArray("no-store"));
        // Login and signup have separate budgets; unrelated routes still work.
        QCOMPARE(request(port_, "/auth/signup", "not-json", true).status, 400);
        QCOMPARE(request(port_, "/").status, 200);
        QCOMPARE(request(port_, "/auth/logout", "{}", true).status, 200);
    }

    void signup_quota_runs_before_database_work_and_recovers_at_expiry() {
        using namespace Infrastructure::Security;
        auto now = LocalRateLimiter::Clock::time_point{};
        auto limiter = std::make_unique<LocalRateLimiter>([&] { return now; });
        MongoDBDataManager database;
        Server server(database, nullptr, std::move(limiter));
        QVERIFY(server.is_listening());
        for (int i = 0; i < 5; ++i)
            QCOMPARE(request(port_, "/auth/signup", "not-json", true).status, 400);
        now += std::chrono::seconds(299);
        const auto blocked = request(port_, "/auth/signup", credentials, true, "192.0.2.1");
        QCOMPARE(blocked.status, 429);
        QCOMPARE(blocked.retry_after, QByteArray("1"));
        QCOMPARE(blocked.cache_control, QByteArray("no-store"));
        now += std::chrono::seconds(1);
        QCOMPARE(request(port_, "/auth/signup", "not-json", true).status, 400);
        QCOMPARE(request(port_, "/auth/login", "not-json", true).status, 400);
    }

    void unavailable_limiter_fails_closed_on_both_auth_routes() {
        using namespace Infrastructure::Security;
        auto limiter = std::make_unique<LocalRateLimiter>();
        for (int i = 0; i < 4096; ++i)
            QCOMPARE(limiter->check(QString::number(i), 1, 300).status, RateLimitStatus::Allowed);
        MongoDBDataManager database;
        Server server(database, nullptr, std::move(limiter));
        QVERIFY(server.is_listening());
        for (const auto& path : {"/auth/login", "/auth/signup"}) {
            const auto response = request(port_, path, "not-json", true);
            QCOMPARE(response.status, 503);
            QVERIFY(response.retry_after.isEmpty());
            QCOMPARE(response.cache_control, QByteArray("no-store"));
        }
    }

    void bearer_sessions_are_independent_and_can_be_revoked() {
        Infrastructure::Http::AuthSessions sessions;
        const auto now = QDateTime::currentDateTimeUtc();
        const auto first = sessions.create("test-user", now);
        const auto second = sessions.create("test-user", now);
        QVERIFY(!first.isEmpty());
        QVERIFY(first != second);
        QVERIFY(sessions.find(first, now));
        QCOMPARE(sessions.find(first, now)->user_id, std::string("test-user"));
        sessions.revoke(first);
        QVERIFY(!sessions.find(first, now));
        QVERIFY(sessions.find(second, now));
        QVERIFY(!sessions.find(second, now.addSecs(8 * 60 * 60)));
    }

    void online_invoice_ignores_stored_logo_path_data() {
        QTest::addColumn<bool>("include_logo_data");
        QTest::newRow("empty-logo-data") << true;
        QTest::newRow("omitted-logo-data") << false;
    }

    void online_invoice_ignores_stored_logo_path() {
        QFETCH(bool, include_logo_data);
        MongoDBDataManager database;
        Infrastructure::Services::InvoiceServices service(database);
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString logoPath = directory.filePath("server-local.png");

        QImage image(16, 16, QImage::Format_RGB32);
        image.fill(Qt::red);
        QVERIFY(image.save(logoPath, "PNG"));

        QJsonObject business{{"BusinessName", "Test business"}, {"LogoPath", logoPath}};
        if (include_logo_data)
            business["LogoData"] = "";
        service.add_business_to_invoice(QJsonDocument(business));
        service.add_client_to_invoice(QJsonDocument(QJsonObject{{"Name", "Test client"}}));

        service.begin_invoice_details(
            QJsonDocument(QJsonObject{{"invoice_number", "TEST-001"}, {"invoice_theme", "PEECE"}}));

        const QJsonDocument items(QJsonArray{QJsonObject{{"Name", "Test item"}, {"Price", 10.0}, {"Quantity", 1}}});

        const QByteArray pdf = service.generate_pdf(items);
        QVERIFY(pdf.startsWith("%PDF-"));
        QFile output("logo-security-test.pdf");
        QVERIFY(output.open(QIODevice::WriteOnly));
        QCOMPARE(output.write(pdf), qint64(pdf.size()));
        output.close();

        qInfo() << "Saved PDF:" << QFileInfo(output).absoluteFilePath();
        // libharu writes image dictionaries outside compressed content streams.
        // This fixture has no other images, so any image is the forbidden fallback.
        QVERIFY2(!pdf.contains("/Subtype /Image"), "A local logo was embedded despite empty LogoData.");
    }

    void invoice_export_reports_failure_and_clears_session() {
        MongoDBDataManager database;
        Infrastructure::Services::InvoiceServices service(database);
        service.add_business_to_invoice(QJsonDocument(QJsonObject{{"BusinessName", "Example business"}}));
        service.add_client_to_invoice(QJsonDocument(QJsonObject{{"Name", "Example client"}}));
        service.begin_invoice_details(QJsonDocument(QJsonObject{{"invoice_number", "TEST-001"}}));
        const QJsonDocument stock(QJsonArray{QJsonObject{{"Name", "Item"}, {"Price", 12.5}, {"Quantity", 2}}});
        QVERIFY(service.has_invoice_details());
        QVERIFY_EXCEPTION_THROWN(service.generate_pdf(QJsonDocument(QJsonArray{})), std::invalid_argument);
        const auto pdf = service.generate_pdf(stock);
        QVERIFY(pdf.startsWith("%PDF-"));
        service.clear_session();
        QVERIFY(!service.has_invoice_details());
        QVERIFY_EXCEPTION_THROWN(service.generate_pdf(stock), std::invalid_argument);
    }

  private:
    quint16 port_ = 0;
    QByteArray original_port_;
    bool had_port_ = false;
    QByteArray original_uri_;
    bool had_uri_ = false;
};

QTEST_GUILESS_MAIN(BackendServerTests)
#include "BackendServerTests.moc"
