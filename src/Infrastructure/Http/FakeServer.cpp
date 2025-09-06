#include "Infrastructure/Http/FakeServer.h"

Server::Server() : InvokeDB(dbHandler.getDatabase()) {

    create_routes_basic();
    create_routes_auth();
    start_server();
}

void Server::create_routes_basic() {
    httpServer_.route("/", []() { return "hello world"; });
}

void Server::create_routes_auth() {
    httpServer_.route("/auth/login", QHttpServerRequest::Method::Post, [](const QHttpServerRequest& request) {
        auto body = request.body();
        // Handle login logic here
        // hit a method in a dbhandler to verify credentials
        qDebug() << "Login attempt with body: " << body;
        return QHttpServerResponse("Logged In", QHttpServerResponder::StatusCode::Accepted);
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
