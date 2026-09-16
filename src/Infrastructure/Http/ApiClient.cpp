#include "Infrastructure/Http/ApiClient.h"
#include "Infrastructure/Http/FakeServer.h"
#include "Domain/Accounts/User.h"

using namespace Infrastructure::Http;

ApiClient::ApiClient(const QUrl& baseUrl, Invoke::Domain::Accounts::IAccountManager* mgr, QObject* parent)
    : networkManager_(new QNetworkAccessManager), baseUrl_(baseUrl), account_manager_(mgr) {}

void Infrastructure::Http::ApiClient::get_business_list() {
    QUrl url = baseUrl_;
    url.setPath("/business/list");
    QNetworkRequest request;
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    try {
        auto reply = networkManager_->get(request);
        connect(reply, &QNetworkReply::finished, this, [=]() {
            if (reply->error() == QNetworkReply::NoError) {
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
                 << "| network error:" << reply->error()
                 << "| error string:" << reply->errorString()
                 << "| HTTP status:"
                 << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()
                 << "| body:" << QString::fromUtf8(response_body);
        if (reply->error() == QNetworkReply::NoError) {
            qDebug() << "[ApiClient::do_signup] Signup succeeded";
            emit signup_succeeded();
        } else {
            QJsonParseError parse_error{};
            const auto response = QJsonDocument::fromJson(response_body, &parse_error);
            const auto message = parse_error.error == QJsonParseError::NoError
                                     ? response.object().value("error").toString()
                                     : QString{};
            qWarning() << "[ApiClient::do_signup] Signup failed"
                       << "| parsed message:" << message
                       << "| parse error:" << parse_error.errorString();
            emit signup_failed(message.isEmpty() ? reply->errorString() : message);
        }
        reply->deleteLater();
    });
}

void Infrastructure::Http::ApiClient::get_stock_list() {
    QUrl url = baseUrl_;
    url.setPath("/stock/list");
    QNetworkRequest request;
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    try {
        auto reply = networkManager_->get(request);
        connect(reply, &QNetworkReply::finished, this, [=]() {
            if (reply->error() == QNetworkReply::NoError) {
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
    current_business_id_ = biz.value("BusinessID").toString();
    QUrl url = baseUrl_;
    url.setPath("/business/objectify");
    QNetworkRequest request;
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    try {
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        QJsonObject json = biz;
        try {
            auto reply = networkManager_->post(request, QJsonDocument(json).toJson());
            connect(reply, &QNetworkReply::finished, this, [=]() {
                if (reply->error() == QNetworkReply::NoError) {
                    QByteArray data = reply->readAll();
                } else {
                    qDebug() << "Network error during business objectification:" << reply->errorString();
                }
                reply->deleteLater();
            });
        } catch (const std::exception& e) {
            qDebug() << "Exception business object request:" << e.what();
        }
    } catch (const std::exception& e) {
        qDebug() << "Exception during business object request:" << e.what();
    }
}

void Infrastructure::Http::ApiClient::invoice_details(const QJsonDocument& invoice) {
    QUrl url = baseUrl_;
    url.setPath("/invoices/invoice_start");
    QNetworkRequest request;
    request.setUrl(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    try {
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        try {
            auto reply = networkManager_->post(request, invoice.toJson());
            connect(reply, &QNetworkReply::finished, this, [=]() {
                if (reply->error() == QNetworkReply::NoError) {
                    QByteArray data = reply->readAll();
                    QJsonDocument jsonResponse = QJsonDocument::fromJson(data);
                } else {
                    qDebug() << "Network error during invoice objectification:" << reply->errorString();
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
    QJsonObject payload = data.object();
    if ((resource == "client" || resource == "stock") && !payload.contains("BusinessID"))
        payload["BusinessID"] = current_business_id_;
    QUrl url = baseUrl_;
    url.setPath("/data/" + resource);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    auto* reply = networkManager_->post(request, QJsonDocument(payload).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply, resource]() {
        const auto body = reply->readAll();
        if (reply->error() == QNetworkReply::NoError) {
            emit resource_saved(resource);
        } else {
            QJsonParseError error{};
            const auto response = QJsonDocument::fromJson(body, &error);
            emit resource_save_failed(response.object().value("error").toString(reply->errorString()));
        }
        reply->deleteLater();
    });
}
void Infrastructure::Http::ApiClient::do_login(const QString& email, const QString& password, bool remember) {
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
            emit login_succeeded();
        } else {
            emit login_failed(reply->errorString());
        }
        reply->deleteLater();
    });
}

// Server listening at: "http://127.0.0.1:1234"
// Sending login reques "http://127.0.0.1:1234/auth/login"
