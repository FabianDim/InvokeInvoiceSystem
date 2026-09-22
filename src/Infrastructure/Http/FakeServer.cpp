#include "Infrastructure/Http/FakeServer.h"
#include "Infrastructure/Database/Mongo/MongoDBDataManager.h"
#include <QJsonArray>
#include <QUrlQuery>
#include "Utils/BusinessLogo.h"

Server::Server(MongoDBDataManager& db_manager, Invoke::Domain::Accounts::IAccountManager* account_manager_)
    : db_manager_(db_manager), account_services_(db_manager, account_manager_), account_manager_(account_manager_),
      invoice_service_(db_manager) {
    create_routes_basic();
    create_routes_auth();
    create_routes_business();
    create_routes_data();
    create_routes_invoices();
}

void Server::create_routes_basic() {
    httpServer_.route("/", []() { return "hello world"; });
}

void Server::create_routes_invoices() {
    httpServer_.route("/invoices/invoice_start",
                      QHttpServerRequest::Method::Post,
                      [this](const QHttpServerRequest& request) -> QHttpServerResponse {
                          if (!account_manager_ || !account_manager_->is_logged_in())
                              return QHttpServerResponse("application/json", "{\"error\":\"unauthorized\"}",
                                                         QHttpServerResponse::StatusCode::Unauthorized);
                          QJsonParseError err{};
                          const QJsonDocument doc = QJsonDocument::fromJson(request.body(), &err);
                          if (err.error != QJsonParseError::NoError || !doc.isObject()) {
                              return QHttpServerResponse(
                                  "Invalid JSON", "text/plain", QHttpServerResponse::StatusCode::BadRequest);
                          }
                          const auto user = account_manager_->getAccount();
                          const auto business_id = doc.object().value("BusinessID").toString();
                          const auto businesses = user ? db_manager_.get_account_businesses(user->getMongoUserID()).object()
                                                       : QJsonObject{};
                          if (business_id.isEmpty() || !businesses.contains(business_id))
                              return QHttpServerResponse("application/json", "{\"error\":\"Choose a business belonging to your account\"}",
                                                         QHttpServerResponse::StatusCode::BadRequest);
                          const auto client_id = doc.object().value("ClientID").toString();
                          QJsonObject selected_client;
                          try {
                              const auto clients = db_manager_.list_resources(
                                  "clients", user->getMongoUserID(), business_id.toStdString());
                              for (const auto& value : clients.array()) {
                                  if (value.toObject().value("ClientID").toString() == client_id && !client_id.isEmpty()) {
                                      selected_client = value.toObject();
                                      break;
                                  }
                              }
                          } catch (const std::exception&) {
                              return QHttpServerResponse("application/json", "{\"error\":\"Could not load the invoice client\"}",
                                                         QHttpServerResponse::StatusCode::InternalServerError);
                          }
                          if (selected_client.isEmpty())
                              return QHttpServerResponse("application/json", "{\"error\":\"Choose a saved client belonging to this business\"}",
                                                         QHttpServerResponse::StatusCode::BadRequest);
                          // Resolve client details from the database, then bind the whole invoice together.
                          invoice_service_.add_business_to_invoice(QJsonDocument(businesses.value(business_id).toObject()));
                          invoice_service_.add_client_to_invoice(QJsonDocument(selected_client));
                          invoice_service_.begin_invoice_details(doc);
                          return QHttpServerResponse("application/json", "{\"ok\":true}", QHttpServerResponse::StatusCode::Ok);
                      });
    httpServer_.route("/invoices/stock-list",
                      QHttpServerRequest::Method::Post,
                      [this](const QHttpServerRequest& request) -> QHttpServerResponse {
                          if (!account_manager_ || !account_manager_->is_logged_in())
                              return QHttpServerResponse("application/json", "{\"error\":\"unauthorized\"}",
                                                         QHttpServerResponse::StatusCode::Unauthorized);
                          QJsonParseError err{};
                          const QJsonDocument doc = QJsonDocument::fromJson(request.body(), &err);
                          if (err.error != QJsonParseError::NoError || !doc.isArray()) {
                              return QHttpServerResponse("application/json", "{\"error\":\"Invalid stock list\"}",
                                                         QHttpServerResponse::StatusCode::BadRequest);
                          }
                          try {
                              const auto path = invoice_service_.add_stock_to_invoice(doc);
                              return QHttpServerResponse(QJsonObject{{"ok", true}, {"path", QString::fromStdString(path)}});
                          } catch (const std::invalid_argument& error) {
                              return QHttpServerResponse(QJsonObject{{"error", QString::fromUtf8(error.what())}},
                                                         QHttpServerResponse::StatusCode::BadRequest);
                          } catch (const std::exception& error) {
                              return QHttpServerResponse(QJsonObject{{"error", QString::fromUtf8(error.what())}},
                                                         QHttpServerResponse::StatusCode::InternalServerError);
                          }
                      });
}

void Server::create_routes_business() {
    httpServer_.route("/business/items", QHttpServerRequest::Method::Get,
                      [this](const QHttpServerRequest& request) -> QHttpServerResponse {
        if (!account_manager_ || !account_manager_->is_logged_in())
            return QHttpServerResponse("application/json", "{\"error\":\"unauthorized\"}",
                                       QHttpServerResponder::StatusCode::Unauthorized);
        const auto user = account_manager_->getAccount();
        const auto business_id = QUrlQuery(request.url()).queryItemValue("BusinessID");
        if (!user || business_id.isEmpty())
            return QHttpServerResponse("application/json", "{\"error\":\"Choose a business first\"}",
                                       QHttpServerResponder::StatusCode::BadRequest);
        try {
            const auto id = business_id.toStdString();
            const auto user_id = user->getMongoUserID();
            const auto businesses = db_manager_.list_resources("businesses", user_id, id).array();
            if (businesses.isEmpty())
                return QHttpServerResponse("application/json", "{\"error\":\"Business is not available for this account\"}",
                                           QHttpServerResponder::StatusCode::Forbidden);
            const QJsonObject items{{"BusinessID", business_id}, {"businesses", businesses},
                                    {"clients", db_manager_.list_resources("clients", user_id, id).array()},
                                    {"stocks", db_manager_.list_resources("stocks", user_id, id).array()}};
            return QHttpServerResponse(items);
        } catch (const std::exception&) {
            return QHttpServerResponse("application/json", "{\"error\":\"Could not load business records\"}",
                                       QHttpServerResponder::StatusCode::InternalServerError);
        }
    });
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
    httpServer_.route("/stock/list", QHttpServerRequest::Method::Get, [this](const QHttpServerRequest& request) -> QHttpServerResponse {
        if (!account_manager_ || !account_manager_->is_logged_in()) {
            return QHttpServerResponse("application/json", "{\"error\":\"unauthorized\"}",
                                       QHttpServerResponder::StatusCode::Unauthorized);
        }
        const auto user = account_manager_->getAccount();
        const auto business_id = QUrlQuery(request.url()).queryItemValue("BusinessID");
        if (!user || business_id.isEmpty())
            return QHttpServerResponse("application/json", "{\"error\":\"Choose a business first\"}",
                                       QHttpServerResponder::StatusCode::BadRequest);
        try {
            const auto stock = db_manager_.list_resources("stocks", user->getMongoUserID(), business_id.toStdString());
            return QHttpServerResponse("application/json", stock.toJson(QJsonDocument::Compact),
                                       QHttpServerResponder::StatusCode::Ok);
        } catch (const std::exception&) {
            return QHttpServerResponse("application/json", "{\"error\":\"Could not load stock\"}",
                                       QHttpServerResponder::StatusCode::InternalServerError);
        }
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
    httpServer_.route("/auth/logout", QHttpServerRequest::Method::Post, [this]() {
        clear_session();
        return QHttpServerResponse(QJsonObject{{"ok", true}});
    });
    httpServer_.route("/auth/login", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest& request) {
        // A backend may outlive its desktop client. Never inherit the previous account's draft.
        clear_session();
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(request.body(), &parseError);
        const auto obj = doc.object();
        if (parseError.error != QJsonParseError::NoError || !doc.isObject() ||
            obj.value("email").toString().trimmed().isEmpty() || obj.value("password").toString().isEmpty())
            return QHttpServerResponse(QJsonObject{{"error", "Enter an email and password"}},
                                       QHttpServerResponder::StatusCode::BadRequest);
        if (!account_manager_)
            return QHttpServerResponse(QJsonObject{{"error", "Account service is unavailable"}},
                                       QHttpServerResponder::StatusCode::ServiceUnavailable);
        try {
            account_manager_->login(obj.value("email").toString().toStdString(),
                                    obj.value("password").toString().toStdString());
            if (account_manager_->is_logged_in())
                return QHttpServerResponse("application/json", "{\"ok\":true}", QHttpServerResponder::StatusCode::Ok);
        } catch (const std::exception&) {
            clear_session();
            return QHttpServerResponse(QJsonObject{{"error", "Could not sign in. Check the backend database connection."}},
                                       QHttpServerResponder::StatusCode::InternalServerError);
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
        if (resource == "business" && !data.object().value("LogoData").toString().isEmpty()) {
            const auto encoded = data.object().value("LogoData").toString().toLatin1();
            QByteArray png;
            QString message = "Choose a PNG or JPEG logo no larger than 5 MB.";
            if (encoded.size() > (BusinessLogo::maximum_bytes + 2) / 3 * 4 ||
                !BusinessLogo::normalise(QByteArray::fromBase64(encoded), png, message))
                return QHttpServerResponse(QJsonObject{{"error", message}}, QHttpServerResponder::StatusCode::BadRequest);
        }
        QString saved_id;
        if (!user || !db_manager_.save_resource(resource.toStdString(), data.object(), user->getMongoUserID(), &saved_id))
            return QHttpServerResponse("application/json", "{\"error\":\"Could not save resource\"}",
                                       QHttpServerResponder::StatusCode::InternalServerError);
        return QHttpServerResponse(QJsonObject{{"ok", true}, {"id", saved_id}});
    };
    httpServer_.route("/data/business", QHttpServerRequest::Method::Post,
                      [save](const QHttpServerRequest& request) { return save("business", request); });
    httpServer_.route("/data/client", QHttpServerRequest::Method::Post,
                      [save](const QHttpServerRequest& request) { return save("client", request); });
    httpServer_.route("/data/stock", QHttpServerRequest::Method::Post,
                      [save](const QHttpServerRequest& request) { return save("stock", request); });
}

void Server::clear_session() {
    invoice_service_.clear_session();
    if (account_manager_)
        account_manager_->logOut();
}

bool Server::start(quint16 port) {
    if (is_listening())
        return true;
    error_string_.clear();
    auto* listener = new QTcpServer(&httpServer_);
    if (!listener->listen(QHostAddress::LocalHost, port)) {
        error_string_ = listener->errorString();
        delete listener;
        return false;
    }
    if (!httpServer_.bind(listener)) {
        error_string_ = "Could not bind the HTTP server to its listener.";
        delete listener;
        return false;
    }
    tcpServer_ = listener;
    qInfo().noquote() << "Backend listening at http://127.0.0.1:" + QString::number(tcpServer_->serverPort());
    return true;
}

bool Server::is_listening() const {
    return tcpServer_ && tcpServer_->isListening();
}

quint16 Server::port() const {
    return is_listening() ? tcpServer_->serverPort() : 0;
}
