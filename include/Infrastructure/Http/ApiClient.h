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
    void get_business_list();

  signals:
    void business_list_received(const QJsonArray& list);

  private:
    bool loginInProgress_ = false;
    Invoke::Domain::Accounts::IAccountManager* account_manager_;
    QNetworkAccessManager* networkManager_;
    QUrl baseUrl_;
};
} // namespace Infrastructure::Http