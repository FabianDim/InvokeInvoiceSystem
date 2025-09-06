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

  private:
    bool loginInProgress_ = false;
    QNetworkAccessManager* networkManager_;
    QUrl baseUrl_;
};
} // namespace Infrastructure::Http