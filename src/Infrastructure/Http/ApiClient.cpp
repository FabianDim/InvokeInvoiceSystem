#include "Infrastructure/Logging/HttpLogging.h"
#include "Infrastructure/Http/ApiClient.h"
#include <QJsonArray>
#include <QUrlQuery>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QSaveFile>

using namespace Infrastructure::Http;

ApiClient::ApiClient(const QUrl& baseUrl, Invoke::Domain::Accounts::IAccountManager* mgr, QObject* parent)
    : QObject(parent), networkManager_(new QNetworkAccessManager(this)), baseUrl_(baseUrl), account_manager_(mgr) {
    connect(this, &ApiClient::login_succeeded, this, []() { qCDebug(Infrastructure::Logging::auth) << "event=client.login_succeeded"; });
    connect(this, &ApiClient::login_failed, this, []() { qCWarning(Infrastructure::Logging::auth) << "event=client.login_failed"; });
    connect(this, &ApiClient::signup_succeeded, this, []() { qCDebug(Infrastructure::Logging::auth) << "event=client.signup_succeeded"; });
    connect(this, &ApiClient::signup_failed, this, []() { qCWarning(Infrastructure::Logging::auth) << "event=client.signup_failed"; });
    connect(this, &ApiClient::business_list_received, this, []() { qCDebug(Infrastructure::Logging::http) << "event=client.business_list_received"; });
    connect(this, &ApiClient::stock_list_received, this, []() { qCDebug(Infrastructure::Logging::http) << "event=client.stock_list_received"; });
    connect(this, &ApiClient::business_items_received, this, []() { qCDebug(Infrastructure::Logging::http) << "event=client.business_items_received"; });
    connect(this, &ApiClient::business_items_failed, this, []() { qCWarning(Infrastructure::Logging::http) << "event=client.business_items_failed"; });
    connect(this, &ApiClient::resource_saved, this, []() { qCDebug(Infrastructure::Logging::http) << "event=client.resource_saved"; });
    connect(this, &ApiClient::resource_save_failed, this, []() { qCWarning(Infrastructure::Logging::http) << "event=client.resource_save_failed"; });
    connect(this, &ApiClient::invoice_stock_saved, this, []() { qCDebug(Infrastructure::Logging::http) << "event=client.invoice_stock_saved"; });
    connect(this, &ApiClient::invoice_stock_save_failed, this, []() { qCWarning(Infrastructure::Logging::http) << "event=client.invoice_stock_save_failed"; });
    connect(this, &ApiClient::invoice_started, this, []() { qCDebug(Infrastructure::Logging::pdf) << "event=client.invoice_started"; });
    connect(this, &ApiClient::invoice_failed, this, []() { qCWarning(Infrastructure::Logging::pdf) << "event=client.invoice_failed"; });
    connect(this, &ApiClient::pdf_generated, this, []() { qCDebug(Infrastructure::Logging::pdf) << "event=client.pdf_generated"; });
    connect(this, &ApiClient::pdf_failed, this, []() { qCWarning(Infrastructure::Logging::pdf) << "event=client.pdf_failed"; });
}

QNetworkRequest ApiClient::make_request(const QUrl& url) const {
    QNetworkRequest request(url);
    request.setRawHeader("X-Request-ID", Infrastructure::Logging::newRequestId());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setTransferTimeout(30000);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);
    if (!session_token_.isEmpty())
        request.setRawHeader("Authorization", "Bearer " + session_token_);
    return request;
}

void ApiClient::invalidate_business_items() {
    Infrastructure::Logging::Operation operation(Infrastructure::Logging::http(), "ApiClient.invalidate_business_items");
    ++business_items_generation_;
    business_items_cache_ = QJsonDocument{};
    business_items_age_.invalidate();
    if (business_items_reply_) {
        qCDebug(Infrastructure::Logging::http) << "event=business_cache.request_aborted";
        business_items_reply_->abort();
        business_items_reply_ = nullptr;
    }
}

void ApiClient::set_offline(bool offline) {
    Infrastructure::Logging::Operation operation(Infrastructure::Logging::http(), "ApiClient.set_offline");
    qCInfo(Infrastructure::Logging::app) << "event=mode.changed offline=" << offline;
    offline_ = offline;
    clear_session();
}

void ApiClient::clear_session() {
    Infrastructure::Logging::Operation operation(Infrastructure::Logging::http(), "ApiClient.clear_session");
    ++invoice_generation_;
    invoice_directory_.clear();
    invoice_filename_.clear();
    const auto old_token = session_token_;
    session_token_.clear();
    if (account_manager_)
        account_manager_->logOut();
    offline_session_ = {};
    loginInProgress_ = false;
    // Disconnect before aborting: no late online response may populate a demo.
    for (auto* reply : networkManager_->findChildren<QNetworkReply*>()) {
        qCDebug(Infrastructure::Logging::http) << "event=client.request.cancelled request_id=" << reply->request().rawHeader("X-Request-ID");
        reply->disconnect(this);
        reply->abort();
        reply->deleteLater();
    }
    business_items_reply_ = nullptr;
    ++session_generation_;
    current_business_id_.clear();
    invalidate_business_items();
    if (!old_token.isEmpty()) {
        QUrl url = baseUrl_;
        url.setPath("/auth/logout");
        auto request = make_request(url);
        request.setRawHeader("Authorization", "Bearer " + old_token);
        auto* reply = networkManager_->post(request, QByteArray("{}"));
        Infrastructure::Logging::traceReply(reply);
        connect(reply, &QNetworkReply::finished, reply, &QObject::deleteLater);
    }
}

void ApiClient::get_business_items(bool refresh) {
    Infrastructure::Logging::Operation operation(Infrastructure::Logging::http(), "ApiClient.get_business_items");
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
        qCDebug(Infrastructure::Logging::http) << "event=business_cache.hit age_ms=" << business_items_age_.elapsed();
        emit business_items_received(business_items_cache_);
        return;
    }
    if (business_items_reply_) {
        qCDebug(Infrastructure::Logging::http) << "event=business_cache.request_coalesced";
        return;
    }
    const auto business_id = current_business_id_;
    const auto generation = business_items_generation_;
    QUrl url = baseUrl_;
    url.setPath("/business/items");
    QUrlQuery query;
    query.addQueryItem("BusinessID", business_id);
    url.setQuery(query);
    auto request = make_request(url);
    request.setTransferTimeout(30000);
    auto* reply = networkManager_->get(request);
    Infrastructure::Logging::traceReply(reply);
    business_items_reply_ = reply;
    connect(reply, &QNetworkReply::finished, this, [this, reply, business_id, generation]() {
        Infrastructure::Logging::RequestContext context(reply->request().rawHeader("X-Request-ID"));
        reply->deleteLater();
        // An old request must never repopulate a new business or account.
        if (generation != business_items_generation_ || business_id != current_business_id_) {
            qCDebug(Infrastructure::Logging::http) << "event=client.response.discarded reason=stale_generation";
            return;
        }
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
        qCDebug(Infrastructure::Logging::http) << "event=business_cache.updated businesses=" << object.value("businesses").toArray().size()
            << "clients=" << object.value("clients").toArray().size() << "stocks=" << object.value("stocks").toArray().size();
        business_items_cache_ = data;
        business_items_age_.start();
        emit business_items_received(data);
    });
}

void Infrastructure::Http::ApiClient::get_business_list() {
    Infrastructure::Logging::Operation operation(Infrastructure::Logging::http(), "ApiClient.get_business_list");
    if (offline_) {
        emit business_list_received(QJsonDocument(offline_session_.businesses));
        return;
    }
    const auto session = session_generation_;
    QUrl url = baseUrl_;
    url.setPath("/business/list");
    auto request = make_request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    try {
        auto reply = networkManager_->get(request);
        Infrastructure::Logging::traceReply(reply);
        connect(reply, &QNetworkReply::finished, this, [=]() {
        Infrastructure::Logging::RequestContext context(reply->request().rawHeader("X-Request-ID"));
            if (reply->error() == QNetworkReply::NoError && session == session_generation_) {
                QByteArray data = reply->readAll();
                QJsonDocument jsonResponse = QJsonDocument::fromJson(data);
                emit business_list_received(jsonResponse);
            } else {
                qCWarning(Infrastructure::Logging::http) << "event=client.response.ignored_or_failed";
            }
            reply->deleteLater(); // Clean up the reply object
        });
    } catch (const std::exception& e) {
        qCWarning(Infrastructure::Logging::http) << "event=business_list.request.exception";
    }
}

void Infrastructure::Http::ApiClient::do_signup(const QJsonDocument& details) {
    Infrastructure::Logging::Operation operation(Infrastructure::Logging::http(), "ApiClient.do_signup");
    if (offline_) {
        emit signup_failed("Exit offline mode to create an account.");
        return;
    }
    QUrl url = baseUrl_;
    url.setPath("/auth/signup");
    auto request = make_request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    qCDebug(Infrastructure::Logging::http) << "[ApiClient::do_signup] Sending signup request to";
    auto* reply = networkManager_->post(request, details.toJson(QJsonDocument::Compact));
    Infrastructure::Logging::traceReply(reply);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        Infrastructure::Logging::RequestContext context(reply->request().rawHeader("X-Request-ID"));
        const auto response_body = reply->readAll();
        qCDebug(Infrastructure::Logging::http) << "[ApiClient::do_signup] Signup response received";
        if (reply->error() == QNetworkReply::NoError) {
            qCDebug(Infrastructure::Logging::http) << "[ApiClient::do_signup] Signup succeeded";
            emit signup_succeeded();
        } else {
            QJsonParseError parse_error{};
            const auto response = QJsonDocument::fromJson(response_body, &parse_error);
            const auto message =
                parse_error.error == QJsonParseError::NoError ? response.object().value("error").toString() : QString{};
            qCWarning(Infrastructure::Logging::http) << "[ApiClient::do_signup] Signup failed";
            emit signup_failed(message.isEmpty() ? reply->errorString() : message);
        }
        reply->deleteLater();
    });
}

void Infrastructure::Http::ApiClient::get_stock_list() {
    Infrastructure::Logging::Operation operation(Infrastructure::Logging::http(), "ApiClient.get_stock_list");
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
    auto request = make_request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    try {
        auto reply = networkManager_->get(request);
        Infrastructure::Logging::traceReply(reply);
        connect(reply, &QNetworkReply::finished, this, [=]() {
        Infrastructure::Logging::RequestContext context(reply->request().rawHeader("X-Request-ID"));
            if (reply->error() == QNetworkReply::NoError && current_business_id_ == business_id) {
                const auto data = reply->readAll();
                emit stock_list_received(QJsonDocument::fromJson(data));
            } else {
                qCWarning(Infrastructure::Logging::http) << "event=client.response.ignored_or_failed";
            }
            reply->deleteLater();
        });
    } catch (const std::exception& e) {
        qCWarning(Infrastructure::Logging::http) << "event=stock_list.request.exception";
    }
}

void Infrastructure::Http::ApiClient::business_selected(const QJsonObject& biz) {
    Infrastructure::Logging::Operation operation(Infrastructure::Logging::http(), "ApiClient.business_selected");
    const auto id = biz.value("BusinessID").toString();
    if (id != current_business_id_) {
        ++invoice_generation_;
        invoice_directory_.clear();
        invoice_filename_.clear();
        qCDebug(Infrastructure::Logging::http) << "event=business.changed invoice_reset=true";
        current_business_id_ = id;
        if (offline_)
            offline_session_.clear_invoice();
        invalidate_business_items();
    }
}

void Infrastructure::Http::ApiClient::invoice_details(const QJsonDocument& invoice) {
    Infrastructure::Logging::Operation operation(Infrastructure::Logging::http(), "ApiClient.invoice_details");
    const auto generation = ++invoice_generation_;
    invoice_directory_.clear();
    invoice_filename_.clear();
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
    const auto directory = payload.take("file_dir").toString();
    const auto filename = payload.take("file_name").toString();
    QUrl url = baseUrl_;
    url.setPath("/invoices/invoice_start");
    auto request = make_request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    try {
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        try {
            auto reply = networkManager_->post(request, QJsonDocument(payload).toJson());
            Infrastructure::Logging::traceReply(reply);
            connect(reply, &QNetworkReply::finished, this, [=]() {
        Infrastructure::Logging::RequestContext context(reply->request().rawHeader("X-Request-ID"));
                reply->deleteLater();
                if (generation != invoice_generation_) {
                    qCDebug(Infrastructure::Logging::http) << "event=client.response.discarded reason=stale_generation";
                    return;
                }
                if (reply->error() == QNetworkReply::NoError) {
                    invoice_directory_ = directory;
                    invoice_filename_ = filename;
                    emit invoice_started();
                } else {
                    const auto response = QJsonDocument::fromJson(reply->readAll());
                    emit invoice_failed(response.object().value("error").toString(reply->errorString()));
                }
            });
        } catch (const std::exception& e) {
            qCWarning(Infrastructure::Logging::http) << "event=invoice.request.exception";
        }
    } catch (const std::exception& e) {
        qCWarning(Infrastructure::Logging::http) << "event=invoice.request.exception";
    }
}

void Infrastructure::Http::ApiClient::stock_list(const QJsonDocument& stock) {
    Infrastructure::Logging::Operation operation(Infrastructure::Logging::http(), "ApiClient.stock_list");
    if (offline_) {
        try {
            emit pdf_generated(offline_session_.generate_pdf(stock.array()));
        } catch (const std::exception& error) {
            emit pdf_failed(QString::fromUtf8(error.what()));
        }
        return;
    }
    qCDebug(Infrastructure::Logging::pdf) << "event=pdf.download.requested item_count=" << stock.array().size();
    auto filename = invoice_filename_.trimmed();
    if (filename.isEmpty() || filename.contains(QRegularExpression("[<>:\"/\\\\|?*]")) || filename == "." || filename == "..") {
        qCWarning(Infrastructure::Logging::pdf) << "event=pdf.export.rejected reason=invalid_filename";
        emit pdf_failed("Enter a file name without path separators or reserved characters in the invoice details.");
        return;
    }
    if (invoice_directory_.isEmpty() || !QFileInfo(invoice_directory_).isDir() || !QFileInfo(invoice_directory_).isWritable()) {
        qCWarning(Infrastructure::Logging::pdf) << "event=pdf.export.rejected reason=unwritable_directory";
        emit pdf_failed("The PDF folder is unavailable. Choose an existing, writable folder in the invoice details.");
        return;
    }
    filename.replace(' ', '_');
    if (!filename.endsWith(".pdf", Qt::CaseInsensitive))
        filename += ".pdf";
    const auto path = QDir(invoice_directory_).filePath(filename);
    const auto generation = invoice_generation_;
    QUrl url = baseUrl_;
    url.setPath("/invoices/stock-list");
    auto request = make_request(url);
    request.setRawHeader("Accept", "application/pdf");
    auto* reply = networkManager_->post(request, stock.toJson(QJsonDocument::Compact));
    Infrastructure::Logging::traceReply(reply);
    connect(reply, &QNetworkReply::finished, this, [this, reply, path, generation]() {
        Infrastructure::Logging::RequestContext context(reply->request().rawHeader("X-Request-ID"));
        reply->deleteLater();
        if (generation != invoice_generation_) {
            qCDebug(Infrastructure::Logging::http) << "event=client.response.discarded reason=stale_generation";
            return;
        }
        const auto data = reply->readAll();
        if (reply->error() != QNetworkReply::NoError) {
            const auto error = QJsonDocument::fromJson(data).object().value("error").toString();
            emit pdf_failed(error.isEmpty() ? "Could not download the PDF: " + reply->errorString() : error);
            return;
        }
        if (!reply->header(QNetworkRequest::ContentTypeHeader).toString().startsWith("application/pdf") ||
            !data.startsWith("%PDF-")) {
            qCWarning(Infrastructure::Logging::pdf) << "event=pdf.download.invalid_content";
            emit pdf_failed("The backend did not return a PDF. Please update the backend and try again.");
            return;
        }
        // Commit atomically so a failed download or disk write cannot replace an existing PDF.
        qCDebug(Infrastructure::Logging::pdf) << "event=pdf.local_write.started bytes=" << data.size();
        QSaveFile file(path);
        if (!file.open(QIODevice::WriteOnly) || file.write(data) != data.size() || !file.commit()) {
            qCWarning(Infrastructure::Logging::pdf) << "event=pdf.local_write.failed code=" << static_cast<int>(file.error());
            emit pdf_failed("Could not save PDF to " + path + ": " + file.errorString());
            return;
        }
        qCDebug(Infrastructure::Logging::pdf) << "event=pdf.local_write.completed bytes=" << data.size();
        emit pdf_generated(path);
    });
}
void Infrastructure::Http::ApiClient::save_resource(const QString& resource, const QJsonDocument& data) {
    post_resource(resource, data);
}

void ApiClient::save_invoice_stock(const QJsonDocument& item, quint64 request_id) {
    Infrastructure::Logging::Operation operation(Infrastructure::Logging::http(), "ApiClient.save_invoice_stock");
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
    Infrastructure::Logging::Operation operation(Infrastructure::Logging::http(), "ApiClient.post_resource");
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
    qCDebug(Infrastructure::Logging::http) << "event=resource.save.requested resource=" << resource;
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
    auto request = make_request(url);
    request.setTransferTimeout(30000);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    auto* reply = networkManager_->post(request, QJsonDocument(payload).toJson(QJsonDocument::Compact));
    Infrastructure::Logging::traceReply(reply);
    connect(reply,
            &QNetworkReply::finished,
            this,
            [this, reply, resource, business_id, session, invoice_item, request_id]() {
        Infrastructure::Logging::RequestContext context(reply->request().rawHeader("X-Request-ID"));
                reply->deleteLater();
                if (session != session_generation_) {
                    qCDebug(Infrastructure::Logging::http) << "event=client.response.discarded reason=stale_generation";
                    return;
                }
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
    Infrastructure::Logging::Operation operation(Infrastructure::Logging::http(), "ApiClient.do_login");
    if (offline_) {
        emit login_failed("Exit offline mode to log in.");
        return;
    }
    if (loginInProgress_) {
        qCDebug(Infrastructure::Logging::auth) << "event=login.ignored reason=already_pending";
        return;
    }
    clear_session();
    loginInProgress_ = true;
    const auto session = session_generation_;

    QUrl url = baseUrl_;
    url.setPath("/auth/login");
    qCDebug(Infrastructure::Logging::http) << "Sending login request to";
    auto request = make_request(url);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setTransferTimeout(30000);

    QJsonObject json;
    json["email"] = email;
    json["password"] = password;
    json["remember"] = remember;
    auto* reply = networkManager_->post(request, QJsonDocument(json).toJson());
    Infrastructure::Logging::traceReply(reply);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        Infrastructure::Logging::RequestContext context(reply->request().rawHeader("X-Request-ID"));
        loginInProgress_ = false;
        const auto response = QJsonDocument::fromJson(reply->readAll()).object();
        if (reply->error() == QNetworkReply::NoError) {
            const auto token = response.value("token").toString().toLatin1();
            const auto identity = response.value("user").toObject();
            if (!response.value("ok").toBool() || token.isEmpty() || identity.value("id").toString().isEmpty()) {
                emit login_failed("The server returned an invalid sign-in response. Please update the backend.");
                reply->deleteLater();
                return;
            }
            qCDebug(Infrastructure::Logging::auth) << "event=client.session.accepted";
            session_token_ = token;
            auto user = std::make_shared<User>(identity.value("email").toString().toStdString(), "");
            user->setMongoUserID(identity.value("id").toString().toStdString());
            if (account_manager_)
                account_manager_->set_authenticated_user(std::move(user));
            emit login_succeeded();
        } else {
            auto message = response.value("error").toString();
            if (message.isEmpty()) {
                message = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 401
                    ? "Invalid email or password" : reply->errorString();
            }
            emit login_failed(message);
        }
    });
}

// Server listening at: "http://127.0.0.1:1234"
// Sending login reques "http://127.0.0.1:1234/auth/login"
