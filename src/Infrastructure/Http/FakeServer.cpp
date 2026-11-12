#include "Infrastructure/Logging/HttpLogging.h"
#include "Infrastructure/Http/FakeServer.h"
#include "Infrastructure/Database/Mongo/MongoDBDataManager.h"
#include <QJsonArray>
#include <QUrlQuery>
#include "Utils/BusinessLogo.h"

Server::Server(MongoDBDataManager& db_manager, Invoke::Domain::Accounts::IAccountManager*)
    : db_manager_(db_manager) {
    qCInfo(Infrastructure::Logging::app) << "event=server.routes.registering";
    create_routes_basic();
    create_routes_auth();
    create_routes_business();
    create_routes_data();
    create_routes_invoices();
    listening_ = start_server() == 0;
    qCInfo(Infrastructure::Logging::app) << "event=server.initialized listening=" << listening_;
}

std::shared_ptr<Infrastructure::Http::AuthSessions::Session> Server::session_for(const QHttpServerRequest& request) {
    const auto authorization = request.value("Authorization");
    if (!authorization.startsWith("Bearer ")) {
        qCWarning(Infrastructure::Logging::auth) << "event=auth.rejected reason=missing_bearer";
        return {};
    }
    return sessions_.find(authorization.mid(7));
}

void Server::create_routes_basic() {
    httpServer_.route("/", Infrastructure::Logging::tracedRoute("/", [](const QHttpServerRequest&) { return QHttpServerResponse("hello world"); }));
}

void Server::create_routes_invoices() {
    httpServer_.route("/invoices/invoice_start",
                      QHttpServerRequest::Method::Post,
                      Infrastructure::Logging::tracedRoute("/invoices/invoice_start", [this](const QHttpServerRequest& request) -> QHttpServerResponse {
                          const auto session = session_for(request);
                          if (!session)
                              return QHttpServerResponse("application/json", "{\"error\":\"unauthorized\"}",
                                                         QHttpServerResponse::StatusCode::Unauthorized);
                          QJsonParseError err{};
                          const QJsonDocument doc = QJsonDocument::fromJson(request.body(), &err);
                          if (err.error != QJsonParseError::NoError || !doc.isObject()) {
                              return QHttpServerResponse(
                                  "Invalid JSON", "text/plain", QHttpServerResponse::StatusCode::BadRequest);
                          }
                          const auto business_id = doc.object().value("BusinessID").toString();
                          const auto client_id = doc.object().value("ClientID").toString();
                          if (!Infrastructure::Database::parse_object_id(business_id.toStdString()) ||
                              !Infrastructure::Database::parse_object_id(client_id.toStdString()))
                              return QHttpServerResponse(QJsonObject{{"error", "Invalid business or client ID"}},
                                                         QHttpServerResponse::StatusCode::BadRequest);
                          const auto businesses = db_manager_.get_account_businesses(session->user_id).object();
                          if (business_id.isEmpty() || !businesses.contains(business_id))
                              return QHttpServerResponse("application/json", "{\"error\":\"Choose a business belonging to your account\"}",
                                                         QHttpServerResponse::StatusCode::BadRequest);
                          QJsonObject selected_client;
                          try {
                              const auto clients = db_manager_.list_resources(
                                  "clients", session->user_id, business_id.toStdString());
                              for (const auto& value : clients.array()) {
                                  if (value.toObject().value("_id").toString() == client_id && !client_id.isEmpty()) {
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
                          session->invoice = std::make_shared<Infrastructure::Services::InvoiceServices>(db_manager_);
                          session->invoice->add_business_to_invoice(QJsonDocument(businesses.value(business_id).toObject()));
                          session->invoice->add_client_to_invoice(QJsonDocument(selected_client));
                          session->invoice->begin_invoice_details(doc);
                          qCDebug(Infrastructure::Logging::pdf) << "event=invoice.session.ready";
                          return QHttpServerResponse("application/json", "{\"ok\":true}", QHttpServerResponse::StatusCode::Ok);
                      }));
    httpServer_.route("/invoices/stock-list",
                      QHttpServerRequest::Method::Post,
                      Infrastructure::Logging::tracedRoute("/invoices/stock-list", [this](const QHttpServerRequest& request) -> QHttpServerResponse {
                          const auto session = session_for(request);
                          if (!session)
                              return QHttpServerResponse(QJsonObject{{"error", "Please sign in again."}},
                                                         QHttpServerResponse::StatusCode::Unauthorized);
                          if (!session->invoice)
                              return QHttpServerResponse(QJsonObject{{"error", "Start an invoice first."}},
                                                         QHttpServerResponse::StatusCode::BadRequest);
                          QJsonParseError err{};
                          const QJsonDocument doc = QJsonDocument::fromJson(request.body(), &err);
                          if (err.error != QJsonParseError::NoError || !doc.isArray())
                              return QHttpServerResponse(QJsonObject{{"error", "Expected an invoice item list."}},
                                                         QHttpServerResponse::StatusCode::BadRequest);
                          try {
                              QHttpServerResponse response("application/pdf", session->invoice->generate_pdf(doc));
                              auto headers = response.headers();
                              headers.append("Cache-Control", "no-store");
                              response.setHeaders(std::move(headers));
                              return response;
                          } catch (const std::invalid_argument& error) {
                              return QHttpServerResponse(QJsonObject{{"error", QString::fromUtf8(error.what())}},
                                                         QHttpServerResponse::StatusCode::BadRequest);
                          } catch (const std::exception&) {
                              return QHttpServerResponse(QJsonObject{{"error", "Could not generate the invoice PDF. Please try again."}},
                                                         QHttpServerResponse::StatusCode::InternalServerError);
                          }
                      }));
}

void Server::create_routes_business() {
    httpServer_.route("/business/items", QHttpServerRequest::Method::Get,
                      Infrastructure::Logging::tracedRoute("/business/items", [this](const QHttpServerRequest& request) -> QHttpServerResponse {
        const auto session = session_for(request);
        if (!session)
            return QHttpServerResponse("application/json", "{\"error\":\"unauthorized\"}",
                                       QHttpServerResponder::StatusCode::Unauthorized);
        const auto business_id = QUrlQuery(request.url()).queryItemValue("BusinessID");
        if (business_id.isEmpty())
            return QHttpServerResponse("application/json", "{\"error\":\"Choose a business first\"}",
                                       QHttpServerResponder::StatusCode::BadRequest);
        try {
            const auto id = business_id.toStdString();
            const auto user_id = session->user_id;
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
    }));
    httpServer_.route("/business/list", QHttpServerRequest::Method::Get, Infrastructure::Logging::tracedRoute("/business/list", [this](const QHttpServerRequest& request) -> QHttpServerResponse {
        const auto session = session_for(request);
        if (!session) {
            QJsonObject err{{"error", "unauthorized"}};
            QJsonDocument doc(err);
            return QHttpServerResponse(
                "application/json", doc.toJson(QJsonDocument::Compact), QHttpServerResponse::StatusCode::Unauthorized);
        }
        const QJsonDocument listJson = db_manager_.get_account_businesses(session->user_id);
        return QHttpServerResponse(
            "application/json", listJson.toJson(QJsonDocument::Compact), QHttpServerResponse::StatusCode::Ok);
    }));
    httpServer_.route("/business/objectify", QHttpServerRequest::Method::Post, Infrastructure::Logging::tracedRoute("/business/objectify", [&](const QHttpServerRequest& request) {
        const auto session = session_for(request);
        if (!session) {
            QJsonObject err{{"error", "unauthorized"}};
            QJsonDocument doc(err);
            return QHttpServerResponse("No authorised user", QHttpServerResponder::StatusCode::BadRequest);
        }
        try {
            QJsonParseError parseError;
            QJsonDocument doc = QJsonDocument::fromJson(request.body(), &parseError);
            const auto business_id = doc.object().value("BusinessID").toString();
            const auto businesses = db_manager_.get_account_businesses(session->user_id).object();
            if (!businesses.contains(business_id))
                return QHttpServerResponse(QJsonObject{{"error", "Business is not available for this account"}},
                                           QHttpServerResponder::StatusCode::Forbidden);
            session->invoice = std::make_shared<Infrastructure::Services::InvoiceServices>(db_manager_);
            session->invoice->add_business_to_invoice(QJsonDocument(businesses.value(business_id).toObject()));
            return QHttpServerResponse("Posting the business", QHttpServerResponder::StatusCode::Ok);
        } catch (const std::exception& e) {
            qCDebug(Infrastructure::Logging::http) << "Exception setting active business:";
            return QHttpServerResponse("Failed to set active business",
                                       QHttpServerResponder::StatusCode::InternalServerError);
        }
    }));
    httpServer_.route("/stock/list", QHttpServerRequest::Method::Get, Infrastructure::Logging::tracedRoute("/stock/list", [this](const QHttpServerRequest& request) -> QHttpServerResponse {
        const auto session = session_for(request);
        if (!session) {
            return QHttpServerResponse("application/json", "{\"error\":\"unauthorized\"}",
                                       QHttpServerResponder::StatusCode::Unauthorized);
        }
        const auto business_id = QUrlQuery(request.url()).queryItemValue("BusinessID");
        if (business_id.isEmpty())
            return QHttpServerResponse("application/json", "{\"error\":\"Choose a business first\"}",
                                       QHttpServerResponder::StatusCode::BadRequest);
        try {
            const auto stock = db_manager_.list_resources("stocks", session->user_id, business_id.toStdString());
            return QHttpServerResponse("application/json", stock.toJson(QJsonDocument::Compact),
                                       QHttpServerResponder::StatusCode::Ok);
        } catch (const std::exception&) {
            return QHttpServerResponse("application/json", "{\"error\":\"Could not load stock\"}",
                                       QHttpServerResponder::StatusCode::InternalServerError);
        }
    }));
}
void Server::create_routes_auth() {
    httpServer_.route("/auth/signup", QHttpServerRequest::Method::Post, Infrastructure::Logging::tracedRoute("/auth/signup", [this](const QHttpServerRequest& request) {
        qCDebug(Infrastructure::Logging::http) << "[Server::signup] Request received";
        QJsonParseError parseError;
        const auto doc = QJsonDocument::fromJson(request.body(), &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
            qCWarning(Infrastructure::Logging::http) << "[Server::signup] Invalid signup JSON";
            return QHttpServerResponse("application/json", "{\"error\":\"Invalid signup data\"}",
                                       QHttpServerResponder::StatusCode::BadRequest);
        }

        const auto obj = doc.object();
        qCDebug(Infrastructure::Logging::http) << "[Server::signup] Parsed signup payload";
        const auto result = db_manager_.create_account(obj);
        switch (result) {
        case MongoDBDataManager::AccountCreationResult::Created:
            qCDebug(Infrastructure::Logging::http) << "[Server::signup] Account created";
            return QHttpServerResponse("application/json", "{\"ok\":true}", QHttpServerResponder::StatusCode::Created);
        case MongoDBDataManager::AccountCreationResult::InvalidInput:
            qCWarning(Infrastructure::Logging::http) << "[Server::signup] Account creation rejected: invalid input";
            return QHttpServerResponse("application/json", "{\"error\":\"Invalid signup data\"}",
                                       QHttpServerResponder::StatusCode::BadRequest);
        case MongoDBDataManager::AccountCreationResult::EmailExists:
            qCWarning(Infrastructure::Logging::http) << "[Server::signup] Account creation rejected: email already exists";
            return QHttpServerResponse("application/json", "{\"error\":\"Email already exists\"}",
                                       QHttpServerResponder::StatusCode::Conflict);
        case MongoDBDataManager::AccountCreationResult::DatabaseError:
            qCWarning(Infrastructure::Logging::http) << "[Server::signup] Account creation rejected: database error";
            return QHttpServerResponse(
                "application/json",
                "{\"error\":\"Database error while creating account. Check MongoDB connection credentials.\"}",
                QHttpServerResponder::StatusCode::InternalServerError);
        }
        return QHttpServerResponse("application/json", "{\"error\":\"Unknown signup error\"}",
                                   QHttpServerResponder::StatusCode::InternalServerError);
    }));
    httpServer_.route("/auth/login", QHttpServerRequest::Method::Post, Infrastructure::Logging::tracedRoute("/auth/login", [this](const QHttpServerRequest& request) {
        QJsonParseError parseError;
        const auto doc = QJsonDocument::fromJson(request.body(), &parseError);
        if (parseError.error != QJsonParseError::NoError || !doc.isObject())
            return QHttpServerResponse(QJsonObject{{"error", "Invalid login request"}},
                                       QHttpServerResponder::StatusCode::BadRequest);
        const auto email = doc.object().value("email").toString().trimmed();
        const auto password = doc.object().value("password").toString();
        try {
            const auto user_id = db_manager_.authenticate_account(email.toStdString(), password.toStdString());
            if (!user_id)
                return QHttpServerResponse(QJsonObject{{"error", "Invalid email or password"}},
                                           QHttpServerResponder::StatusCode::Unauthorized);
            const auto token = sessions_.create(*user_id);
            if (token.isEmpty())
                return QHttpServerResponse(QJsonObject{{"error", "Sign-in is busy. Please try again later."}},
                                           QHttpServerResponder::StatusCode::ServiceUnavailable);
            auto response = QHttpServerResponse(QJsonObject{
                {"ok", true}, {"token", QString::fromLatin1(token)},
                {"user", QJsonObject{{"_id", QString::fromStdString(*user_id)}, {"email", email}}}});
            auto headers = response.headers();
            headers.append("Cache-Control", "no-store");
            response.setHeaders(std::move(headers));
            return response;
        } catch (const std::exception&) {
            qCWarning(Infrastructure::Logging::http) << "[Server::login] Database authentication lookup failed";
            return QHttpServerResponse(QJsonObject{{"error", "The server could not connect to the database. Please try again later."}},
                                       QHttpServerResponder::StatusCode::ServiceUnavailable);
        }
    }));
    httpServer_.route("/auth/logout", QHttpServerRequest::Method::Post, Infrastructure::Logging::tracedRoute("/auth/logout", [this](const QHttpServerRequest& request) {
        const auto authorization = request.value("Authorization");
        if (authorization.startsWith("Bearer "))
            sessions_.revoke(authorization.mid(7));
        return QHttpServerResponse(QJsonObject{{"ok", true}});
    }));
}

void Server::create_routes_data() {
    const auto save = [this](const QString& resource, const QHttpServerRequest& request) {
        const auto session = session_for(request);
        if (!session)
            return QHttpServerResponse("application/json", "{\"error\":\"unauthorized\"}",
                                       QHttpServerResponder::StatusCode::Unauthorized);
        QJsonParseError error{};
        const auto data = QJsonDocument::fromJson(request.body(), &error);
        if (error.error != QJsonParseError::NoError || !data.isObject())
            return QHttpServerResponse("application/json", "{\"error\":\"Invalid JSON\"}",
                                       QHttpServerResponder::StatusCode::BadRequest);
        if (resource == "business" && !data.object().value("LogoData").toString().isEmpty()) {
            const auto encoded = data.object().value("LogoData").toString().toLatin1();
            QByteArray png;
            QString message = "Choose a PNG or JPEG logo no larger than 5 MB.";
            if (encoded.size() > (BusinessLogo::maximum_bytes + 2) / 3 * 4 ||
                !BusinessLogo::normalise(QByteArray::fromBase64(encoded), png, message))
                return QHttpServerResponse(QJsonObject{{"error", message}}, QHttpServerResponder::StatusCode::BadRequest);
        }
        if (resource != "business" && !Infrastructure::Database::parse_object_id(
                data.object().value("BusinessID").toString().toStdString()))
            return QHttpServerResponse(QJsonObject{{"error", "Invalid business ID"}},
                                       QHttpServerResponder::StatusCode::BadRequest);
        QString saved_id;
        if (!db_manager_.save_resource(resource.toStdString(), data.object(), session->user_id, &saved_id))
            return QHttpServerResponse("application/json", "{\"error\":\"Could not save resource\"}",
                                       QHttpServerResponder::StatusCode::InternalServerError);
        return QHttpServerResponse(QJsonObject{{"ok", true}, {"_id", saved_id}});
    };
    httpServer_.route("/data/business", QHttpServerRequest::Method::Post,
                      Infrastructure::Logging::tracedRoute("/data/business", [save](const QHttpServerRequest& request) { return save("business", request); }));
    httpServer_.route("/data/client", QHttpServerRequest::Method::Post,
                      Infrastructure::Logging::tracedRoute("/data/client", [save](const QHttpServerRequest& request) { return save("client", request); }));
    httpServer_.route("/data/stock", QHttpServerRequest::Method::Post,
                      Infrastructure::Logging::tracedRoute("/data/stock", [save](const QHttpServerRequest& request) { return save("stock", request); }));
}

int Server::start_server() {
    Infrastructure::Logging::Operation operation(Infrastructure::Logging::http(), "Server.start_server");
    QHostAddress host = QHostAddress::LocalHost;
    quint16 port = 1234;
    if (qEnvironmentVariableIsSet("PORT")) {
        bool valid = false;
        const int configuredPort = qgetenv("PORT").toInt(&valid);
        if (!valid || configuredPort < 1 || configuredPort > 65535) {
            qCCritical(Infrastructure::Logging::http) << "PORT must be an integer between 1 and 65535; server was not started.";
            return -1;
        }
        // Heroku routes HTTP traffic to the assigned port on all IPv4 interfaces.
        host = QHostAddress::AnyIPv4;
        port = static_cast<quint16>(configuredPort);
    }

    auto* tcpServer = new QTcpServer(&httpServer_);
    if (!tcpServer->listen(host, port)) {
        qCCritical(Infrastructure::Logging::http) << "event=server.listen.failed address=" << host.toString()
            << "port=" << port << "socket_error=" << static_cast<int>(tcpServer->serverError());
        delete tcpServer;
        return -1;
    }
    if (!httpServer_.bind(tcpServer)) {
        qCCritical(Infrastructure::Logging::http) << "event=server.bind.failed address=" << host.toString() << "port=" << port;
        delete tcpServer;
        return -1;
    }
    qCInfo(Infrastructure::Logging::http) << "event=server.listening address=" << host.toString() << "port=" << port;
    return 0;
}
