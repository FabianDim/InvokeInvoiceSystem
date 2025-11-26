#include "Infrastructure/Http/FakeServer.h"
#include "Infrastructure/Database/Mongo/MongoDBDataManager.h"

Server::Server(MongoDBDataManager& db_manager, Invoke::Domain::Accounts::IAccountManager* account_manager_)
    : db_manager_(db_manager), account_services_(db_manager, account_manager_), account_manager_(account_manager_),
      invoice_service_(db_manager) {
    create_routes_basic();
    create_routes_auth();
    create_routes_business();
    create_routes_invoices();
    start_server();
}

void Server::create_routes_basic() {
    httpServer_.route("/", []() { return "hello world"; });
}

void Server::create_routes_invoices() {
    httpServer_.route("/invoices/invoice_start",
                      QHttpServerRequest::Method::Post,
                      [this](const QHttpServerRequest& request) -> QHttpServerResponse {
                          QJsonParseError err{};
                          const QJsonDocument doc = QJsonDocument::fromJson(request.body(), &err);
                          if (err.error != QJsonParseError::NoError) {
                              return QHttpServerResponse(
                                  "Invalid JSON", "text/plain", QHttpServerResponse::StatusCode::BadRequest);
                          }

                          invoice_service_.begin_invoice_details(doc);
                          return QHttpServerResponse("Invalid JSON", "text/plain", QHttpServerResponse::StatusCode::Ok);
                      });
    httpServer_.route("/invoices/stock-list",
                      QHttpServerRequest::Method::Post,
                      [this](const QHttpServerRequest& request) -> QHttpServerResponse {
                          QJsonParseError err{};
                          const QJsonDocument doc = QJsonDocument::fromJson(request.body(), &err);
                          if (err.error != QJsonParseError::NoError) {
                              return QHttpServerResponse(
                                  "Invalid JSON", "text/plain", QHttpServerResponse::StatusCode::BadRequest);
                          }

                          invoice_service_.add_stock_to_invoice(doc);
                          return QHttpServerResponse("Invalid JSON", "text/plain", QHttpServerResponse::StatusCode::Ok);
                      });
}

void Server::create_routes_business() {
    httpServer_.route("/business/list", QHttpServerRequest::Method::Get, [this]() -> QHttpServerResponse {
        if (!account_manager_ || !account_manager_->is_logged_in()) {
            QJsonObject err{{"error", "unauthorized"}};
            QJsonDocument doc(err);
            return QHttpServerResponse(
                "application/json", doc.toJson(QJsonDocument::Compact), QHttpServerResponse::StatusCode::Unauthorized);
        }
        const QJsonDocument listJson = account_services_.fetch_account_businesses();
        return QHttpServerResponse(
            "application/json", listJson.toJson(QJsonDocument::Compact), QHttpServerResponse::StatusCode::Ok);
    });
    httpServer_.route("/business/objectify", QHttpServerRequest::Method::Post, [&](const QHttpServerRequest& request) {
        if (!account_manager_ || !account_manager_->is_logged_in()) {
            QJsonObject err{{"error", "unauthorized"}};
            QJsonDocument doc(err);
            return QHttpServerResponse("No authorised user", QHttpServerResponder::StatusCode::BadRequest);
        }
        try {
            QJsonParseError parseError;
            QJsonDocument doc = QJsonDocument::fromJson(request.body(), &parseError);
            invoice_service_.add_business_to_invoice(doc);
            return QHttpServerResponse("Posting the business", QHttpServerResponder::StatusCode::Ok);
        } catch (const std::exception& e) {
            qDebug() << "Exception setting active business:" << e.what();
            return QHttpServerResponse("Failed to set active business",
                                       QHttpServerResponder::StatusCode::InternalServerError);
        }
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
    auto* sslServer = new QTcpServer(&httpServer_);
    if (!sslServer->listen(host, 1234) || !httpServer_.bind(sslServer)) {
        delete sslServer;
        return -1;
    }
    QString url = QString("http://%1:%2").arg(sslServer->serverAddress().toString()).arg(sslServer->serverPort());

    qDebug() << "🗄 Server listening at:" << url;
    return 0;
}
