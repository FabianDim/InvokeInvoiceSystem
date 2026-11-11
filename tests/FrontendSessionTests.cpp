#include "Application/Accounts/FrontendAccountSession.h"
#include "Domain/Accounts/User.h"
#include "Infrastructure/Http/ApiClient.h"
#include <QHttpServer>
#include <QHttpServerResponder>
#include <QJsonArray>
#include <QTcpServer>
#include <QtTest>

using Application::Accounts::FrontendAccountSession;
using Infrastructure::Http::ApiClient;
using StatusCode = QHttpServerResponder::StatusCode;

namespace {
struct SessionServer {
    QHttpServer http;
    QTcpServer* tcp = new QTcpServer(&http);
    QByteArray login_body = "{\"ok\":true}";
    StatusCode login_status = StatusCode::Ok;
    QByteArray pdf_body = "{\"ok\":true,\"path\":\"C:/invoices/example.pdf\"}";
    StatusCode pdf_status = StatusCode::Ok;
    bool hold_login = false;
    bool hold_logout = false;
    std::unique_ptr<QHttpServerResponder> pending_login;
    std::unique_ptr<QHttpServerResponder> pending_logout;
    QStringList requests;

    bool start() {
        http.route("/auth/login", QHttpServerRequest::Method::Post,
                   [this](QHttpServerResponder& responder) {
                       requests.append("login");
                       if (hold_login)
                           pending_login = std::make_unique<QHttpServerResponder>(std::move(responder));
                       else
                           responder.write(login_body, "application/json", login_status);
                   });
        http.route("/auth/logout", QHttpServerRequest::Method::Post,
                   [this](QHttpServerResponder& responder) {
                       requests.append("logout");
                       if (hold_logout)
                           pending_logout = std::make_unique<QHttpServerResponder>(std::move(responder));
                       else
                           responder.write(QJsonDocument(QJsonObject{{"ok", true}}));
                   });
        http.route("/invoices/stock-list", QHttpServerRequest::Method::Post, [this]() {
            requests.append("pdf");
            return QHttpServerResponse("application/json", pdf_body, pdf_status);
        });
        return tcp->listen(QHostAddress::LocalHost, 0) && http.bind(tcp);
    }

    void finish_login() {
        pending_login->write(login_body, "application/json", login_status);
        pending_login.reset();
    }

    void finish_logout() {
        pending_logout->write(QJsonDocument(QJsonObject{{"ok", true}}));
        pending_logout.reset();
    }

    QUrl url() const { return QUrl(QString("http://127.0.0.1:%1").arg(tcp->serverPort())); }
};
} // namespace

class FrontendSessionTests : public QObject {
    Q_OBJECT

  private slots:
    void successful_http_login_updates_frontend_without_storing_password() {
        SessionServer server;
        QVERIFY(server.start());
        FrontendAccountSession account;
        ApiClient api(server.url(), &account);
        QVERIFY(!account.is_logged_in());
        QSignalSpy logged_in(&api, &ApiClient::login_succeeded);
        QSignalSpy failed(&api, &ApiClient::login_failed);
        api.do_login("test@example.com", "Secret1!", false);
        QTRY_COMPARE(logged_in.count(), 1);
        QVERIFY(account.is_logged_in());
        QCOMPARE(account.getAccount()->getEmail(), std::string("test@example.com"));
        QVERIFY(account.getAccount()->getPassword().empty());
        QVERIFY(failed.isEmpty());
        QCOMPARE(server.requests, QStringList{"login"});
    }

    void failed_login_does_not_authenticate_data() {
        QTest::addColumn<QByteArray>("body");
        QTest::addColumn<int>("status");
        QTest::addColumn<QString>("message");
        QTest::newRow("rejected") << QByteArray("{\"error\":\"Invalid email or password\"}") << 401
                                  << QString("Invalid email or password");
        QTest::newRow("malformed") << QByteArray("not json") << 200
                                   << QString("The backend returned an invalid login response.");
        QTest::newRow("missing-success") << QByteArray("{}") << 200
                                         << QString("The backend returned an invalid login response.");
    }

    void failed_login_does_not_authenticate() {
        QFETCH(QByteArray, body);
        QFETCH(int, status);
        QFETCH(QString, message);
        SessionServer server;
        server.login_body = body;
        server.login_status = static_cast<StatusCode>(status);
        QVERIFY(server.start());
        FrontendAccountSession account;
        account.login("previous@example.com", "");
        ApiClient api(server.url(), &account);
        QSignalSpy logged_in(&api, &ApiClient::login_succeeded);
        QSignalSpy failed(&api, &ApiClient::login_failed);
        api.do_login("test@example.com", "wrong", false);
        QTRY_COMPARE(failed.count(), 1);
        QCOMPARE(failed.first().first().toString(), message);
        QVERIFY(!account.is_logged_in());
        QVERIFY(logged_in.isEmpty());
        QTRY_COMPARE(server.requests, (QStringList{"login", "logout"}));
    }

    void sign_out_clears_local_and_backend_sessions() {
        SessionServer server;
        QVERIFY(server.start());
        FrontendAccountSession account;
        ApiClient api(server.url(), &account);
        QSignalSpy logged_in(&api, &ApiClient::login_succeeded);
        api.do_login("test@example.com", "Secret1!", false);
        QTRY_COMPARE(logged_in.count(), 1);
        api.clear_session();
        QVERIFY(!account.is_logged_in());
        QVERIFY(!account.getAccount());
        QTRY_COMPARE(server.requests, (QStringList{"login", "logout"}));
    }

    void new_login_waits_for_backend_logout() {
        SessionServer server;
        QVERIFY(server.start());
        FrontendAccountSession account;
        ApiClient api(server.url(), &account);
        QSignalSpy logged_in(&api, &ApiClient::login_succeeded);
        api.do_login("first@example.com", "Secret1!", false);
        QTRY_COMPARE(logged_in.count(), 1);
        server.hold_logout = true;
        api.clear_session();
        api.do_login("second@example.com", "Secret2!", false);
        QTRY_VERIFY(server.pending_logout != nullptr);
        QCOMPARE(server.requests, (QStringList{"login", "logout"}));
        QVERIFY(!account.is_logged_in());
        server.finish_logout();
        QTRY_COMPARE(logged_in.count(), 2);
        QCOMPARE(server.requests, (QStringList{"login", "logout", "login"}));
        QCOMPARE(account.getAccount()->getEmail(), std::string("second@example.com"));
    }

    void entering_demo_ends_online_session() {
        SessionServer server;
        QVERIFY(server.start());
        FrontendAccountSession account;
        ApiClient api(server.url(), &account);
        QSignalSpy logged_in(&api, &ApiClient::login_succeeded);
        api.do_login("test@example.com", "Secret1!", false);
        QTRY_COMPARE(logged_in.count(), 1);
        api.set_offline(true);
        QVERIFY(api.is_offline());
        QVERIFY(!account.is_logged_in());
        QTRY_COMPARE(server.requests, (QStringList{"login", "logout"}));
    }

    void pending_login_cannot_authenticate_after_entering_demo() {
        SessionServer server;
        server.hold_login = true;
        QVERIFY(server.start());
        FrontendAccountSession account;
        ApiClient api(server.url(), &account);
        QSignalSpy logged_in(&api, &ApiClient::login_succeeded);
        api.do_login("test@example.com", "Secret1!", false);
        QTRY_VERIFY(server.pending_login != nullptr);
        api.set_offline(true);
        server.finish_login();
        QTRY_COMPARE(server.requests, (QStringList{"login", "logout"}));
        QVERIFY(api.is_offline());
        QVERIFY(!account.is_logged_in());
        QVERIFY(logged_in.isEmpty());
    }

    void fresh_offline_session_needs_no_backend() {
        SessionServer server;
        QVERIFY(server.start());
        QSignalSpy connections(server.tcp, &QTcpServer::newConnection);
        FrontendAccountSession account;
        ApiClient api(server.url(), &account);
        QSignalSpy businesses(&api, &ApiClient::business_list_received);
        api.set_offline(true);
        api.get_business_list();
        QCOMPARE(businesses.count(), 1);
        api.set_offline(false);
        QTest::qWait(50);
        QVERIFY(connections.isEmpty());
        QVERIFY(server.requests.isEmpty());
    }

    void pdf_results_cross_the_http_boundary_data() {
        QTest::addColumn<QByteArray>("body");
        QTest::addColumn<int>("status");
        QTest::addColumn<bool>("succeeds");
        QTest::addColumn<QString>("result");
        QTest::newRow("created") << QByteArray("{\"ok\":true,\"path\":\"C:/invoices/example.pdf\"}") << 200
                                 << true << QString("C:/invoices/example.pdf");
        QTest::newRow("generation-failed") << QByteArray("{\"error\":\"Output folder is unavailable\"}") << 500
                                           << false << QString("Output folder is unavailable");
        QTest::newRow("invalid-response") << QByteArray("{}") << 200
                                          << false << QString("The backend returned an invalid PDF response.");
    }

    void pdf_results_cross_the_http_boundary() {
        QFETCH(QByteArray, body);
        QFETCH(int, status);
        QFETCH(bool, succeeds);
        QFETCH(QString, result);
        SessionServer server;
        server.pdf_body = body;
        server.pdf_status = static_cast<StatusCode>(status);
        QVERIFY(server.start());
        FrontendAccountSession account;
        ApiClient api(server.url(), &account);
        QSignalSpy generated(&api, &ApiClient::pdf_generated);
        QSignalSpy failed(&api, &ApiClient::pdf_failed);
        api.stock_list(QJsonDocument(QJsonArray{}));
        QTRY_COMPARE(generated.count() + failed.count(), 1);
        QCOMPARE(generated.count(), succeeds ? 1 : 0);
        QCOMPARE((succeeds ? generated : failed).first().first().toString(), result);
    }
};

QTEST_GUILESS_MAIN(FrontendSessionTests)
#include "FrontendSessionTests.moc"
