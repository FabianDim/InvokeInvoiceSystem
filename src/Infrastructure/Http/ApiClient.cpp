#include "Infrastructure/Http/ApiClient.h"
#include "Infrastructure/Http/FakeServer.h"
#include "Domain/Accounts/User.h"
#include <QUrlQuery>

using namespace Infrastructure::Http;

ApiClient::ApiClient(const QUrl& baseUrl, Invoke::Domain::Accounts::IAccountManager* mgr, QObject* parent)
    : QObject(parent), networkManager_(new QNetworkAccessManager(this)), baseUrl_(baseUrl), account_manager_(mgr) {}

void ApiClient::invalidate_business_items() {
    ++business_items_generation_;
    business_items_cache_ = QJsonDocument{};
    business_items_age_.invalidate();
    if (business_items_reply_) {
        business_items_reply_->abort();
        business_items_reply_ = nullptr;
    }
}

void ApiClient::set_offline(bool offline) {
    offline_ = offline;
    clear_session();
}

void ApiClient::clear_session() {
    offline_session_ = {};
    loginInProgress_ = false;
    // Disconnect before aborting: no late online response may populate a demo.
    for (auto* reply : networkManager_->findChildren<QNetworkReply*>()) {
        reply->disconnect(this);
        reply->abort();
        reply->deleteLater();
    }
    business_items_reply_ = nullptr;
    ++session_generation_;
    current_business_id_.clear();
    invalidate_business_items();
}

void ApiClient::get_business_items(bool refresh) {
    if (offline_) {
        emit business_items_received(QJsonDocument(offline_session_.items(current_business_id_)));
        return;
    }
    if (current_business_id_.isEmpty()) {
        emit business_items_failed("Choose a business on the home page first.");
        return;
    }
    if (refresh)
        invalidate_business_items();
    // Cache only the active business in memory for five minutes. Switching
    // businesses, signing out, or saving records discards the snapshot.
    if (!business_items_cache_.isNull() && business_items_age_.isValid() && business_items_age_.elapsed() < 300000) {
        emit business_items_received(business_items_cache_);
        return;
    }
    if (business_items_reply_)
        return;
    const auto business_id = current_business_id_;
    const auto generation = business_items_generation_;
    QUrl url = baseUrl_;
    url.setPath("/business/items");
    QUrlQuery query;
    query.addQueryItem("BusinessID", business_id);
    url.setQuery(query);
    QNetworkRequest request(url);
    request.setTransferTimeout(30000);
    auto* reply = networkManager_->get(request);
    business_items_reply_ = reply;
    connect(reply, &QNetworkReply::finished, this, [this, reply, business_id, generation]() {
        reply->deleteLater();
        // An old request must never repopulate a new business or account.
        if (generation != business_items_generation_ || business_id != current_business_id_)
            return;
        business_items_reply_ = nullptr;
        QJsonParseError error{};
        const auto data = QJsonDocument::fromJson(reply->readAll(), &error);
        if (reply->error() != QNetworkReply::NoError) {
            emit business_items_failed(data.object().value("error").toString(reply->errorString()));
            return;
        }
        const auto object = data.object();
        if (error.error != QJsonParseError::NoError || !data.isObject() ||
            object.value("BusinessID").toString() != business_id || !object.value("businesses").isArray() ||
            !object.value("clients").isArray() || !object.value("stocks").isArray()) {
            emit business_items_failed("The server returned invalid business records.");
            return;
        }
        business_items_cache_ = data;
        business_items_age_.start();
        emit business_items_received(data);
    });
}

void Infrastructure::Http::ApiClient::get_business_list() {
    if (offline_) {
        emit business_list_received(QJsonDocument(offline_session_.businesses));
        return;
    }
    const auto session = session_generation_;
    QUrl url = baseUrl_;
    url.setPath("/business/list");
    QNetworkRequest request;
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    try {
        auto reply = networkManager_->get(request);
        connect(reply, &QNetworkReply::finished, this, [=]() {
            if (reply->error() == QNetworkReply::NoError && session == session_generation_) {
                QByteArray data = reply->readAll();
                QJsonDocument jsonResponse = QJsonDocument::fromJson(data);
                emit business_list_received(jsonResponse);
            } else {
                qDebug() << "Network error:" << reply->errorString();
            }
            reply->deleteLater(); // Clean up the reply object
        });
    } catch (const std::exception& e) {
        qDebug() << "Exception during business list request:" << e.what();
    }
}

void Infrastructure::Http::ApiClient::do_signup(const QJsonDocument& details) {
    if (offline_) {
        emit signup_failed("Exit offline mode to create an account.");
        return;
    }
    QUrl url = baseUrl_;
    url.setPath("/auth/signup");
    QNetworkRequest request;
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    const auto payload = details.object();
    qDebug() << "[ApiClient::do_signup] Sending signup request to" << url.toString()
             << "| email:" << payload.value("email").toString()
             << "| firstName:" << payload.value("firstName").toString()
             << "| lastName:" << payload.value("lastName").toString()
             << "| password length:" << payload.value("password").toString().length();
    auto* reply = networkManager_->post(request, details.toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        const auto response_body = reply->readAll();
        qDebug() << "[ApiClient::do_signup] Signup response received"
                 << "| network error:" << reply->error() << "| error string:" << reply->errorString()
                 << "| HTTP status:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()
                 << "| body:" << QString::fromUtf8(response_body);
        if (reply->error() == QNetworkReply::NoError) {
            qDebug() << "[ApiClient::do_signup] Signup succeeded";
            emit signup_succeeded();
        } else {
            QJsonParseError parse_error{};
            const auto response = QJsonDocument::fromJson(response_body, &parse_error);
            const auto message =
                parse_error.error == QJsonParseError::NoError ? response.object().value("error").toString() : QString{};
            qWarning() << "[ApiClient::do_signup] Signup failed"
                       << "| parsed message:" << message << "| parse error:" << parse_error.errorString();
            emit signup_failed(message.isEmpty() ? reply->errorString() : message);
        }
        reply->deleteLater();
    });
}

void Infrastructure::Http::ApiClient::get_stock_list() {
    if (offline_) {
        emit stock_list_received(QJsonDocument(offline_session_.items(current_business_id_).value("stocks").toArray()));
        return;
    }
    if (current_business_id_.isEmpty()) {
        emit stock_list_received(QJsonDocument(QJsonArray{}));
        return;
    }
    const auto business_id = current_business_id_;
    QUrl url = baseUrl_;
    url.setPath("/stock/list");
    QUrlQuery query;
    query.addQueryItem("BusinessID", business_id);
    url.setQuery(query);
    QNetworkRequest request;
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    try {
        auto reply = networkManager_->get(request);
        connect(reply, &QNetworkReply::finished, this, [=]() {
            if (reply->error() == QNetworkReply::NoError && current_business_id_ == business_id) {
                const auto data = reply->readAll();
                emit stock_list_received(QJsonDocument::fromJson(data));
            } else {
                qDebug() << "Network error:" << reply->errorString();
            }
            reply->deleteLater();
        });
    } catch (const std::exception& e) {
        qDebug() << "Exception during stock list request:" << e.what();
    }
}

void Infrastructure::Http::ApiClient::business_selected(const QJsonObject& biz) {
    const auto id = biz.value("BusinessID").toString();
    if (id != current_business_id_) {
        current_business_id_ = id;
        if (offline_)
            offline_session_.clear_invoice();
        invalidate_business_items();
    }
}

void Infrastructure::Http::ApiClient::invoice_details(const QJsonDocument& invoice) {
    if (offline_) {
        try {
            offline_session_.begin_invoice(invoice.object(), current_business_id_);
            emit invoice_started();
        } catch (const std::exception& error) {
            emit invoice_failed(QString::fromUtf8(error.what()));
        }
        return;
    }
    if (current_business_id_.isEmpty()) {
        emit invoice_failed("Choose a business on the dashboard first.");
        return;
    }
    auto payload = invoice.object();
    if (payload.value("ClientID").toString().isEmpty()) {
        emit invoice_failed("Choose a client for this invoice first.");
        return;
    }
    payload["BusinessID"] = current_business_id_;
    QUrl url = baseUrl_;
    url.setPath("/invoices/invoice_start");
    QNetworkRequest request;
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    try {
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        try {
            auto reply = networkManager_->post(request, QJsonDocument(payload).toJson());
            connect(reply, &QNetworkReply::finished, this, [=]() {
                if (reply->error() == QNetworkReply::NoError) {
                    emit invoice_started();
                } else {
                    const auto response = QJsonDocument::fromJson(reply->readAll());
                    emit invoice_failed(response.object().value("error").toString(reply->errorString()));
                }
                reply->deleteLater();
            });
        } catch (const std::exception& e) {
            qDebug() << "Exception invoice object request:" << e.what();
        }
    } catch (const std::exception& e) {
        qDebug() << "Exception during invoice object request:" << e.what();
    }
}

void Infrastructure::Http::ApiClient::stock_list(const QJsonDocument& stock) {
    if (offline_) {
        try {
            emit pdf_generated(offline_session_.generate_pdf(stock.array()));
        } catch (const std::exception& error) {
            emit pdf_failed(QString::fromUtf8(error.what()));
        }
        return;
    }
    QUrl url = baseUrl_;
    url.setPath("/invoices/stock-list");
    qDebug() << "Sending invoice item list to: " << url.toString();
    QNetworkRequest request;
    request.setUrl(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    try {
        auto reply = networkManager_->post(request, stock.toJson());
    } catch (const std::exception e) {
        qDebug() << "Exception during login request:" << e.what();
    }
}
void Infrastructure::Http::ApiClient::save_resource(const QString& resource, const QJsonDocument& data) {
    post_resource(resource, data);
}

void ApiClient::save_invoice_stock(const QJsonDocument& item, quint64 request_id) {
    auto data = item.object();
    // Invoice quantity and inventory on hand have different meanings.
    data["Quantity"] = data.value("StockOnHand");
    data.remove("StockOnHand");
    data.remove("Notes");
    post_resource("stock", QJsonDocument(data), item, request_id);
}

void ApiClient::post_resource(const QString& resource,
                              const QJsonDocument& data,
                              const QJsonDocument& invoice_item,
                              quint64 request_id) {
    if (offline_) {
        try {
            const auto id = offline_session_.save(resource, data.object(), current_business_id_);
            emit resource_saved(resource);
            if (!invoice_item.isNull()) {
                auto saved = invoice_item.object();
                saved["StockID"] = id;
                emit invoice_stock_saved(QJsonDocument(saved), request_id);
            }
        } catch (const std::exception& error) {
            if (invoice_item.isNull())
                emit resource_save_failed(QString::fromUtf8(error.what()));
            else
                emit invoice_stock_save_failed(QString::fromUtf8(error.what()), request_id);
        }
        return;
    }
    const auto business_id = current_business_id_;
    const auto session = session_generation_;
    QJsonObject payload = data.object();
    if (resource == "client" || resource == "stock") {
        if (current_business_id_.isEmpty()) {
            if (invoice_item.isNull())
                emit resource_save_failed("Choose a business on the dashboard first.");
            else
                emit invoice_stock_save_failed("Choose a business on the dashboard first.", request_id);
            return;
        }
        payload["BusinessID"] = current_business_id_;
    }
    QUrl url = baseUrl_;
    url.setPath("/data/" + resource);
    QNetworkRequest request(url);
    request.setTransferTimeout(30000);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    auto* reply = networkManager_->post(request, QJsonDocument(payload).toJson(QJsonDocument::Compact));
    connect(reply,
            &QNetworkReply::finished,
            this,
            [this, reply, resource, business_id, session, invoice_item, request_id]() {
                reply->deleteLater();
                if (session != session_generation_)
                    return;
                const auto body = reply->readAll();
                if (reply->error() == QNetworkReply::NoError) {
                    if (business_id == current_business_id_)
                        invalidate_business_items();
                    emit resource_saved(resource);
                    if (!invoice_item.isNull() && business_id == current_business_id_) {
                        auto saved = invoice_item.object();
                        saved["StockID"] = QJsonDocument::fromJson(body).object().value("id");
                        emit invoice_stock_saved(QJsonDocument(saved), request_id);
                    }
                } else {
                    QJsonParseError error{};
                    const auto response = QJsonDocument::fromJson(body, &error);
                    const auto message = response.object().value("error").toString(reply->errorString());
                    if (invoice_item.isNull())
                        emit resource_save_failed(message);
                    else if (business_id == current_business_id_)
                        emit invoice_stock_save_failed(message, request_id);
                }
            });
}
void Infrastructure::Http::ApiClient::do_login(const QString& email, const QString& password, bool remember) {
    if (offline_) {
        emit login_failed("Exit offline mode to log in.");
        return;
    }
    if (loginInProgress_)
        return;
    loginInProgress_ = true;

    QUrl url = baseUrl_;
    url.setPath("/auth/login");
    qDebug() << "Sending login request to" << url.toString();
    QNetworkRequest request;
    request.setUrl(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["email"] = email;
    json["password"] = password;
    json["remember"] = remember;
    auto* reply = networkManager_->post(request, QJsonDocument(json).toJson());
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        loginInProgress_ = false;
        if (reply->error() == QNetworkReply::NoError) {
            clear_session();
            emit login_succeeded();
        } else {
            emit login_failed(reply->errorString());
        }
        reply->deleteLater();
    });
}

// Server listening at: "http://127.0.0.1:1234"
// Sending login reques "http://127.0.0.1:1234/auth/login"
