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
};

Response request(quint16 port, const QString& path, const QByteArray& body = {}, bool post = false) {
    QNetworkAccessManager network;
    QNetworkRequest request(QUrl(QString("http://127.0.0.1:%1%2").arg(port).arg(path)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    auto* reply = post ? network.post(request, body) : network.get(request);
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);
    timeout.start(3000);
    loop.exec();
    return {reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt(), reply->readAll()};
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
        QCOMPARE(response.body, QByteArray("hello world"));
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

    void online_invoice_ignores_stored_logo_path() {
        MongoDBDataManager database;
        Infrastructure::Services::InvoiceServices service(database);
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString logoPath = directory.filePath("server-local.png");

        QImage image(16, 16, QImage::Format_RGB32);
        image.fill(Qt::red);
        QVERIFY(image.save(logoPath, "PNG"));

        service.add_business_to_invoice(
            QJsonDocument(QJsonObject{{"BusinessName", "Test business"}, {"LogoPath", logoPath}, {"LogoData", ""}}));
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
