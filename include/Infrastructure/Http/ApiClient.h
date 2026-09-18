#pragma once

#include "pch.h"
#include <QNetworkAccessManager>
#include "Domain/Accounts/Interfaces/IAccountManager.h"
class Server;

namespace Infrastructure::Http {
class ApiClient : public QObject {
    Q_OBJECT
  public:
    explicit ApiClient(const QUrl& baseUrl, Invoke::Domain::Accounts::IAccountManager* mgr, QObject* parent = nullptr);
  public slots:
    void do_login(const QString& email, const QString& password, bool remember);
    void do_signup(const QJsonDocument& details);
    void get_business_list();
    void get_stock_list();
    void business_selected(const QJsonObject& biz);
    void invoice_details(const QJsonDocument& invoice);
    void stock_list(const QJsonDocument& stock);
    void save_resource(const QString& resource, const QJsonDocument& data);
  signals:
    void business_list_received(const QJsonDocument& list);
    void stock_list_received(const QJsonDocument& list);
    void invoice_started();
    void invoice_failed(const QString& message);
    void login_succeeded();
    void login_failed(const QString& message);
    void signup_succeeded();
    void signup_failed(const QString& message);
    void resource_saved(const QString& resource);
    void resource_save_failed(const QString& message);

  private:
    bool loginInProgress_ = false;
    Invoke::Domain::Accounts::IAccountManager* account_manager_;
    QNetworkAccessManager* networkManager_;
    QUrl baseUrl_;
    QString current_business_id_;
};
} // namespace Infrastructure::Http
