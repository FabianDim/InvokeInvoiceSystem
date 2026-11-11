#include <QtTest>
#include <QEventLoop>
#include <QFile>
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
class TestAccountManager final : public Invoke::Domain::Accounts::IAccountManager {
  public:
    bool accept_login = true;
    bool fail_login = false;
    bool logged_in = false;
    bool login_started_logged_in = false;
    bool validEmail(const std::string&) override { return true; }
    bool validName(const std::string&) override { return true; }
    bool doesAccountExist(const std::string&) override { return true; }
    bool doesPasswordMatch(const std::string&) override { return true; }
    bool validatePassword(const std::string&) override { return true; }
    void createAccount(std::string&, std::string&) override {}
    void login(const std::string&, const std::string&) override {
        login_started_logged_in = logged_in;
        if (fail_login)
            throw std::runtime_error("Simulated database failure");
        logged_in = accept_login;
    }
    void logOut() override { logged_in = false; }
    bool is_logged_in() const override { return logged_in; }
    bool needsAccountSetup(const std::string&) override { return false; }
    std::shared_ptr<User> getAccount() override { return nullptr; }
};

struct Response {
    int status;
    QByteArray body;
};

Response request(Server& server, const QString& path, const QByteArray& body = {}, bool post = false) {
    QNetworkAccessManager network;
    QNetworkRequest request(QUrl(QString("http://127.0.0.1:%1%2").arg(server.port()).arg(path)));
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
        // All tests must work without parsing a MongoDB URI or connecting to a database.
        original_uri_ = qgetenv("MONGODB_URI");
        had_uri_ = qEnvironmentVariableIsSet("MONGODB_URI");
        qputenv("MONGODB_URI", "invalid-uri-that-must-not-be-used");
    }

    void cleanupTestCase() {
        if (had_uri_)
            qputenv("MONGODB_URI", original_uri_);
        else
            qunsetenv("MONGODB_URI");
    }

    void starts_explicitly_and_serves_requests() {
        MongoDBDataManager database;
        TestAccountManager account;
        Server server(database, &account);
        QVERIFY(!server.is_listening());
        QCOMPARE(server.port(), quint16(0));
        QVERIFY(server.start(0));
        QVERIFY(server.is_listening());
        QVERIFY(server.port() != 0);
        const auto port = server.port();
        QVERIFY(server.start());
        QCOMPARE(server.port(), port);
        const auto response = request(server, "/");
        QCOMPARE(response.status, 200);
        QCOMPARE(response.body, QByteArray("hello world"));
    }

    void occupied_port_reports_failure_and_can_retry() {
        QTcpServer occupied;
        QVERIFY(occupied.listen(QHostAddress::LocalHost, 0));
        MongoDBDataManager database;
        TestAccountManager account;
        Server server(database, &account);
        QVERIFY(!server.start(occupied.serverPort()));
        QVERIFY(!server.is_listening());
        QCOMPARE(server.port(), quint16(0));
        QVERIFY(!server.error_string().isEmpty());
        QVERIFY(server.start(0));
        QVERIFY(server.error_string().isEmpty());
    }

    void logout_protects_invoice_and_data_routes() {
        MongoDBDataManager database;
        TestAccountManager account;
        Server server(database, &account);
        QVERIFY(server.start(0));
        QCOMPARE(request(server, "/invoices/stock-list", "[]", true).status, 401);
        QCOMPARE(request(server, "/auth/login", credentials, true).status, 200);
        QVERIFY(account.is_logged_in());
        // Authenticated, but no invoice has been started yet.
        QCOMPARE(request(server, "/invoices/stock-list", "[]", true).status, 400);
        QCOMPARE(request(server, "/auth/logout", "{}", true).status, 200);
        QVERIFY(!account.is_logged_in());
        QCOMPARE(request(server, "/auth/logout", "{}", true).status, 200);
        QCOMPARE(request(server, "/invoices/stock-list", "[]", true).status, 401);
        QCOMPARE(request(server, "/invoices/invoice_start", "{}", true).status, 401);
        QCOMPARE(request(server, "/business/list").status, 401);
        QCOMPARE(request(server, "/stock/list").status, 401);
        QCOMPARE(request(server, "/data/business", "{}", true).status, 401);
    }

    void login_replaces_prior_session_and_handles_errors() {
        MongoDBDataManager database;
        TestAccountManager account;
        Server server(database, &account);
        QVERIFY(server.start(0));
        QCOMPARE(request(server, "/auth/login", credentials, true).status, 200);
        QCOMPARE(request(server, "/auth/login", credentials, true).status, 200);
        QVERIFY(!account.login_started_logged_in);
        QCOMPARE(request(server, "/auth/login", "not-json", true).status, 400);
        QVERIFY(!account.is_logged_in());
        account.accept_login = false;
        QCOMPARE(request(server, "/auth/login", credentials, true).status, 401);
        QVERIFY(!account.is_logged_in());
        account.fail_login = true;
        QCOMPARE(request(server, "/auth/login", credentials, true).status, 500);
        QVERIFY(!account.is_logged_in());
        QCOMPARE(request(server, "/").status, 200);
        account.fail_login = false;
        account.accept_login = true;
        QCOMPARE(request(server, "/auth/login", credentials, true).status, 200);
    }

    void invoice_export_reports_failure_and_clears_session() {
        MongoDBDataManager database;
        Infrastructure::Services::InvoiceServices service(database);
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        service.add_business_to_invoice(QJsonDocument(QJsonObject{{"BusinessName", "Example business"}}));
        service.add_client_to_invoice(QJsonDocument(QJsonObject{{"Name", "Example client"}}));
        service.begin_invoice_details(QJsonDocument(QJsonObject{{"file_dir", directory.filePath("missing")},
                                                               {"file_name", "invoice"}}));
        const QJsonDocument stock(QJsonArray{QJsonObject{{"Name", "Item"}, {"Price", 12.5}, {"Quantity", 2}}});
        QVERIFY(service.has_invoice_details());
        QVERIFY_EXCEPTION_THROWN(service.add_stock_to_invoice(stock), std::runtime_error);
        service.begin_invoice_details(QJsonDocument(QJsonObject{{"file_dir", directory.path()}, {"file_name", "invoice"}}));
        const auto path = QString::fromStdString(service.add_stock_to_invoice(stock));
        QCOMPARE(path, directory.filePath("invoice.pdf"));
        QFile pdf(path);
        QVERIFY(pdf.open(QIODevice::ReadOnly));
        QCOMPARE(pdf.read(5), QByteArray("%PDF-"));
        service.clear_session();
        QVERIFY(!service.has_invoice_details());
        QVERIFY_EXCEPTION_THROWN(service.add_stock_to_invoice(stock), std::invalid_argument);
    }

  private:
    QByteArray original_uri_;
    bool had_uri_ = false;
};

QTEST_GUILESS_MAIN(BackendServerTests)
#include "BackendServerTests.moc"
