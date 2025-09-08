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
    tcpServer_ = new QTcpServer(&httpServer_);
    if (!tcpServer_->listen(host, 1234) || !httpServer_.bind(tcpServer_)) {
        delete tcpServer_;
        return -1;
    }
    QString url = QString("http://%1:%2").arg(tcpServer_->serverAddress().toString()).arg(tcpServer_->serverPort());

    qDebug() << "🗄 Server listening at:" << url;
    return 0;
}
