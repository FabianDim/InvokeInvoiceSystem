#include "Infrastructure/Http/FakeServer.h"

Server::Server() : InvokeDB(dbHandler.getDatabase()) {
    start_server();
    create_routes_basic();
}

void Server::create_routes_basic() {
    httpServer_.route("/", []() { return "hello world"; });
}

void Server::create_routes_auth() {}

int Server::start_server() {

    tcpServer_ = new QTcpServer();
    if (!tcpServer_->listen() || !httpServer_.bind(tcpServer_)) {
        delete tcpServer_;
        return -1;
    }
    qDebug() << "Listening on port" << tcpServer_->serverPort();
}

Server::~Server() {
    delete tcpServer_
}
