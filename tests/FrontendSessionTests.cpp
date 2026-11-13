#include "Application/Accounts/FrontendAccountSession.h"
#include "Domain/Accounts/User.h"
#include "Infrastructure/Http/ApiClient.h"
#include <QHttpServer>
#include <QHttpServerResponder>
#include <QJsonArray>
#include <QFile>
#include <QTemporaryDir>
#include <QTcpServer>
#include <QtTest>

using Application::Accounts::FrontendAccountSession;
using Infrastructure::Http::ApiClient;
using StatusCode = QHttpServerResponder::StatusCode;

namespace {
struct SessionServer {
    QHttpServer http;
    QTcpServer* tcp = new QTcpServer(&http);
    QByteArray login_body;
    QByteArray pending_login_body;
    int login_count = 0;
    QList<QByteArray> logout_authorizations;
    StatusCode login_status = StatusCode::Ok;
    QByteArray pdf_body = "%PDF-1.4\ntransport fixture\n%%EOF\n";
    QByteArray pdf_type = "application/pdf";
    StatusCode pdf_status = StatusCode::Ok;
    bool hold_login = false;
    bool hold_logout = false;
    std::unique_ptr<QHttpServerResponder> pending_login;
    std::unique_ptr<QHttpServerResponder> pending_logout;
    QStringList requests;

    bool start() {
        http.route("/auth/login", QHttpServerRequest::Method::Post,
                   [this](const QHttpServerRequest& request, QHttpServerResponder& responder) {
                       requests.append("login");
                       ++login_count;
                       const auto identity = QJsonObject{
                           {"_id", "65d000000000000000000003"},
                           {"email", QJsonDocument::fromJson(request.body()).object().value("email")}};
                       const auto body = login_body.isNull()
                           ? QJsonDocument(QJsonObject{{"ok", true}, {"token", QString("session-%1").arg(login_count)},
                                                       {"user", identity}}).toJson(QJsonDocument::Compact)
                           : login_body;
                       if (hold_login) {
                           pending_login_body = body;
                           pending_login = std::make_unique<QHttpServerResponder>(std::move(responder));
                       } else
                           responder.write(body, "application/json", login_status);
                   });
        http.route("/auth/logout", QHttpServerRequest::Method::Post,
                   [this](const QHttpServerRequest& request, QHttpServerResponder& responder) {
                       requests.append("logout");
                       logout_authorizations.append(request.value("Authorization"));
                       if (hold_logout)
                           pending_logout = std::make_unique<QHttpServerResponder>(std::move(responder));
                       else
                           responder.write(QJsonDocument(QJsonObject{{"ok", true}}));
                   });
        http.route("/invoices/stock-list", QHttpServerRequest::Method::Post, [this]() {
            requests.append("pdf");
            return QHttpServerResponse(pdf_type, pdf_body, pdf_status);
        });
        http.route("/invoices/invoice_start", QHttpServerRequest::Method::Post, []() {
            return QHttpServerResponse(QJsonObject{{"ok", true}});
        });
        return tcp->listen(QHostAddress::LocalHost, 0) && http.bind(tcp);
    }

    void finish_login() {
        pending_login->write(pending_login_body, "application/json", login_status);
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
                                   << QString("The server returned an invalid sign-in response. Please update the backend.");
        QTest::newRow("missing-success") << QByteArray("{}") << 200
                                         << QString("The server returned an invalid sign-in response. Please update the backend.");
        QTest::newRow("missing-token") << QByteArray("{\"ok\":true,\"user\":{\"_id\":\"65d000000000000000000003\"}}") << 200
                                       << QString("The server returned an invalid sign-in response. Please update the backend.");
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
        QCOMPARE(server.requests, QStringList{"login"});
        QVERIFY(server.logout_authorizations.isEmpty()); // No token was issued to revoke.
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
        QCOMPARE(server.logout_authorizations, (QList<QByteArray>{"Bearer session-1"}));
    }

    void old_token_logout_cannot_clear_new_login() {
        SessionServer server;
        QVERIFY(server.start());
        FrontendAccountSession account;
        ApiClient api(server.url(), &account);
        QSignalSpy logged_in(&api, &ApiClient::login_succeeded);
        api.do_login("first@example.com", "Secret1!", false);
        QTRY_COMPARE(logged_in.count(), 1);
        server.hold_logout = true;
        api.clear_session();
        QTRY_VERIFY(server.pending_logout != nullptr);
        QCOMPARE(server.requests, (QStringList{"login", "logout"}));
        QVERIFY(!account.is_logged_in());
        QCOMPARE(server.logout_authorizations, (QList<QByteArray>{"Bearer session-1"}));
        api.do_login("second@example.com", "Secret2!", false);
        QTRY_COMPARE(logged_in.count(), 2);
        QCOMPARE(server.requests, (QStringList{"login", "logout", "login"}));
        QCOMPARE(account.getAccount()->getEmail(), std::string("second@example.com"));
        server.finish_logout();
        QTest::qWait(50);
        QVERIFY(account.is_logged_in());
        QCOMPARE(account.getAccount()->getEmail(), std::string("second@example.com"));
        server.hold_logout = false;
        api.clear_session();
        QTRY_COMPARE(server.logout_authorizations.size(), 2);
        QCOMPARE(server.logout_authorizations.last(), QByteArray("Bearer session-2"));
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
        QTest::qWait(50); // The aborted request's response must not authenticate a demo.
        QCOMPARE(server.requests, QStringList{"login"});
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
        QTest::addColumn<QByteArray>("content_type");
        QTest::addColumn<int>("status");
        QTest::addColumn<bool>("succeeds");
        QTest::addColumn<QString>("result");
        QTest::newRow("created") << QByteArray("%PDF-1.4\ntransport fixture\n%%EOF\n") << QByteArray("application/pdf") << 200
                                 << true << QString{};
        QTest::newRow("generation-failed") << QByteArray("{\"error\":\"Output folder is unavailable\"}") << QByteArray("application/json") << 500
                                           << false << QString("Output folder is unavailable");
        QTest::newRow("legacy-path-response") << QByteArray("{\"ok\":true,\"path\":\"C:/invoices/example.pdf\"}") << QByteArray("application/json") << 200
                                          << false << QString("The backend did not return a PDF. Please update the backend and try again.");
        QTest::newRow("invalid-pdf-body") << QByteArray("not a PDF") << QByteArray("application/pdf") << 200
                                          << false << QString("The backend did not return a PDF. Please update the backend and try again.");
    }

    void pdf_results_cross_the_http_boundary() {
        QFETCH(QByteArray, body);
        QFETCH(QByteArray, content_type);
        QFETCH(int, status);
        QFETCH(bool, succeeds);
        QFETCH(QString, result);
        SessionServer server;
        server.pdf_body = body;
        server.pdf_type = content_type;
        server.pdf_status = static_cast<StatusCode>(status);
        QVERIFY(server.start());
        FrontendAccountSession account;
        ApiClient api(server.url(), &account);
        QTemporaryDir output;
        QVERIFY(output.isValid());
        api.business_selected(QJsonObject{{"_id", "65a000000000000000000001"}});
        QSignalSpy started(&api, &ApiClient::invoice_started);
        api.invoice_details(QJsonDocument(QJsonObject{{"ClientID", "65b000000000000000000001"},
                                                     {"file_dir", output.path()}, {"file_name", "example"}}));
        QTRY_COMPARE(started.count(), 1);
        QSignalSpy generated(&api, &ApiClient::pdf_generated);
        QSignalSpy failed(&api, &ApiClient::pdf_failed);
        api.stock_list(QJsonDocument(QJsonArray{}));
        QTRY_COMPARE(generated.count() + failed.count(), 1);
        QCOMPARE(generated.count(), succeeds ? 1 : 0);
        QCOMPARE((succeeds ? generated : failed).first().first().toString(),
                 succeeds ? output.filePath("example.pdf") : result);
        QCOMPARE(server.requests, QStringList{"pdf"});
        QFile pdf(output.filePath("example.pdf"));
        if (succeeds) {
            QVERIFY(pdf.open(QIODevice::ReadOnly));
            QCOMPARE(pdf.readAll(), body);
        } else {
            QVERIFY(!pdf.exists());
        }
    }
};

QTEST_GUILESS_MAIN(FrontendSessionTests)
#include "FrontendSessionTests.moc"
