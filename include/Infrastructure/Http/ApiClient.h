#pragma once

#include "pch.h"
#include <QNetworkAccessManager>

class Server;

namespace Infrastructure::Http {
class ApiClient : public QObject {
    Q_OBJECT
  public:
    explicit ApiClient(const QUrl& baseUrl, QObject* parent = nullptr);
  public slots:
    void do_login(const QString& email, const QString& password, bool remember);
    std::unordered_map<QString, QString> get_business_list();

  private:
    bool loginInProgress_ = false;
    Invoke::Domain::Accounts::IAccountManager* account_manager_;
    QNetworkAccessManager* networkManager_;
    QUrl baseUrl_;
};
} // namespace Infrastructure::Http