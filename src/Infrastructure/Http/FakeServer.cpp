#include "Infrastructure/Http/FakeServer.h"
#include "Infrastructure/Database/Mongo/MongoDBDataManager.h"

Server::Server(MongoDBDataManager& db_manager) : db_manager_(db_manager), account_services_(db_manager) {
    create_routes_basic();
    create_routes_auth();
    start_server();
}

void Server::create_routes_basic() {
    httpServer_.route("/", []() { return "hello world"; });
}

void Server::create_routes_invoices() {
    httpServer_.route(
        "/invoices/invoice_file", QHttpServerRequest::Method::Post, [&](const QHttpServerRequest& request) {
            QJsonParseError parseError;
            QJsonDocument doc = QJsonDocument::fromJson(request.body(), &parseError);
        });
}

void Server::create_routes_business() {
    httpServer_.route("/business/list", QHttpServerRequest::Method::Get, [&]() {
        if (!account_manager_->is_logged_in()) {
            return;
        }
        const auto& list_json = account_services_.get_account_businesses();
        QHttpServerResponse(list_json.toJson(), "application/json");
    });
}
void Server::create_routes_auth() {
    httpServer_.route("/auth/login", QHttpServerRequest::Method::Post, [&](const QHttpServerRequest& request) {
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(request.body(), &parseError);

        if (account_services_.validate_login(doc)) {
            qDebug() << "Logging In Successfully";
            return QHttpServerResponse("Login Successful", QHttpServerResponder::StatusCode::Ok);
        }
        return QHttpServerResponse("Not logging in", QHttpServerResponder::StatusCode::BadRequest);
    });
}

int Server::start_server() {
    const QHostAddress host = QHostAddress::LocalHost;
    auto* sslServer = new QSslServer(&httpServer_);
    if (!sslServer->listen(host, 1234) || !httpServer_.bind(sslServer)) {
        delete sslServer;
        return -1;
    }
    QString url = QString("http://%1:%2").arg(sslServer->serverAddress().toString()).arg(sslServer->serverPort());

    qDebug() << "🗄 Server listening at:" << url;
    return 0;
}
