#include "Infrastructure/Http/ApiClient.h"
#include "Infrastructure/Http/FakeServer.h"
using namespace Infrastructure::Http;

ApiClient::ApiClient(const QUrl& baseUrl, QObject* parent)
    : networkManager_(new QNetworkAccessManager), baseUrl_(baseUrl) {}

void ApiClient::do_login(const QString& email, const QString& password, bool remember) {
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
        networkManager_->post(request, QJsonDocument(json).toJson());

    } catch (const std::exception& e) {
        qDebug() << "Exception during login request:" << e.what();
    }
}

// Server listening at: "http://127.0.0.1:1234"
// Sending login reques "http://127.0.0.1:1234/auth/login"