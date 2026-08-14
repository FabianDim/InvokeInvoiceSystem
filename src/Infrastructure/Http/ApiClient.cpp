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

void Infrastructure::Http::ApiClient::business_selected(const QJsonObject& biz) {
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
    try {
        auto reply = networkManager_->post(request, QJsonDocument(json).toJson());
    } catch (const std::exception& e) {
        qDebug() << "Exception during login request:" << e.what();
    }
}

// Server listening at: "http://127.0.0.1:1234"
// Sending login reques "http://127.0.0.1:1234/auth/login"