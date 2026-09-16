#include "Infrastructure/Http/FakeServer.h"
#include "Infrastructure/Database/Mongo/MongoDBDataManager.h"

Server::Server(MongoDBDataManager& db_manager, Invoke::Domain::Accounts::IAccountManager* account_manager_)
    : db_manager_(db_manager), account_services_(db_manager, account_manager_), account_manager_(account_manager_),
      invoice_service_(db_manager) {
    create_routes_basic();
    create_routes_auth();
    create_routes_business();
    create_routes_data();
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
    httpServer_.route("/stock/list", QHttpServerRequest::Method::Get, [this]() -> QHttpServerResponse {
        if (!account_manager_ || !account_manager_->is_logged_in()) {
            return QHttpServerResponse("application/json", "{\"error\":\"unauthorized\"}",
                                       QHttpServerResponder::StatusCode::Unauthorized);
        }
        const auto user = account_manager_->getAccount();
        const auto stock = db_manager_.list_resources("stocks", user->getMongoUserID());
        return QHttpServerResponse("application/json", stock.toJson(QJsonDocument::Compact),
                                   QHttpServerResponder::StatusCode::Ok);
    });
}
void Server::create_routes_auth() {
    httpServer_.route("/auth/signup", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest& request) {
        qDebug() << "[Server::signup] Request received"
                 << "| body bytes:" << request.body().size();
        QJsonParseError parseError;
        const auto doc = QJsonDocument::fromJson(request.body(), &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
            qWarning() << "[Server::signup] Invalid signup JSON"
                       << "| parse error:" << parseError.errorString()
                       << "| is object:" << doc.isObject();
            return QHttpServerResponse("application/json", "{\"error\":\"Invalid signup data\"}",
                                       QHttpServerResponder::StatusCode::BadRequest);
        }

        const auto obj = doc.object();
        qDebug() << "[Server::signup] Parsed signup payload"
                 << "| email:" << obj.value("email").toString()
                 << "| firstName:" << obj.value("firstName").toString()
                 << "| lastName:" << obj.value("lastName").toString()
                 << "| password length:" << obj.value("password").toString().length();
        const auto result = db_manager_.create_account(obj);
        switch (result) {
        case MongoDBDataManager::AccountCreationResult::Created:
            qDebug() << "[Server::signup] Account created"
                     << "| email:" << obj.value("email").toString();
            return QHttpServerResponse("application/json", "{\"ok\":true}", QHttpServerResponder::StatusCode::Created);
        case MongoDBDataManager::AccountCreationResult::InvalidInput:
            qWarning() << "[Server::signup] Account creation rejected: invalid input"
                       << "| email:" << obj.value("email").toString();
            return QHttpServerResponse("application/json", "{\"error\":\"Invalid signup data\"}",
                                       QHttpServerResponder::StatusCode::BadRequest);
        case MongoDBDataManager::AccountCreationResult::EmailExists:
            qWarning() << "[Server::signup] Account creation rejected: email already exists"
                       << "| email:" << obj.value("email").toString();
            return QHttpServerResponse("application/json", "{\"error\":\"Email already exists\"}",
                                       QHttpServerResponder::StatusCode::Conflict);
        case MongoDBDataManager::AccountCreationResult::DatabaseError:
            qWarning() << "[Server::signup] Account creation rejected: database error"
                       << "| email:" << obj.value("email").toString();
            return QHttpServerResponse(
                "application/json",
                "{\"error\":\"Database error while creating account. Check MongoDB connection credentials.\"}",
                QHttpServerResponder::StatusCode::InternalServerError);
        }
        return QHttpServerResponse("application/json", "{\"error\":\"Unknown signup error\"}",
                                   QHttpServerResponder::StatusCode::InternalServerError);
    });
    httpServer_.route("/auth/login", QHttpServerRequest::Method::Post, [&](const QHttpServerRequest& request) {
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(request.body(), &parseError);

        if (account_services_.validate_login(doc)) {
            const auto obj = doc.object();
            account_manager_->login(obj.value("email").toString().toStdString(),
                                    obj.value("password").toString().toStdString());
            qDebug() << "Logging In Successfully";
            return QHttpServerResponse("application/json", "{\"ok\":true}", QHttpServerResponder::StatusCode::Ok);
        }
        return QHttpServerResponse("application/json", "{\"error\":\"Invalid email or password\"}",
                                   QHttpServerResponder::StatusCode::Unauthorized);
    });
}

void Server::create_routes_data() {
    const auto save = [this](const QString& resource, const QHttpServerRequest& request) {
        if (!account_manager_ || !account_manager_->is_logged_in())
            return QHttpServerResponse("application/json", "{\"error\":\"unauthorized\"}",
                                       QHttpServerResponder::StatusCode::Unauthorized);
        QJsonParseError error{};
        const auto data = QJsonDocument::fromJson(request.body(), &error);
        if (error.error != QJsonParseError::NoError || !data.isObject())
            return QHttpServerResponse("application/json", "{\"error\":\"Invalid JSON\"}",
                                       QHttpServerResponder::StatusCode::BadRequest);
        const auto user = account_manager_->getAccount();
        if (!user || !db_manager_.save_resource(resource.toStdString(), data.object(), user->getMongoUserID()))
            return QHttpServerResponse("application/json", "{\"error\":\"Could not save resource\"}",
                                       QHttpServerResponder::StatusCode::InternalServerError);
        return QHttpServerResponse("application/json", "{\"ok\":true}", QHttpServerResponder::StatusCode::Ok);
    };
    httpServer_.route("/data/business", QHttpServerRequest::Method::Post,
                      [save](const QHttpServerRequest& request) { return save("business", request); });
    httpServer_.route("/data/client", QHttpServerRequest::Method::Post,
                      [save](const QHttpServerRequest& request) { return save("client", request); });
    httpServer_.route("/data/stock", QHttpServerRequest::Method::Post,
                      [save](const QHttpServerRequest& request) { return save("stock", request); });
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
