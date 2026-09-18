#pragma once

#include "pch.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QElapsedTimer>
#include <QPointer>
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
    void save_invoice_stock(const QJsonDocument& item, quint64 request_id);
    void get_business_items(bool refresh = false);
    void clear_session();
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
    void business_items_received(const QJsonDocument& items);
    void business_items_failed(const QString& message);
    void invoice_stock_saved(const QJsonDocument& item, quint64 request_id);
    void invoice_stock_save_failed(const QString& message, quint64 request_id);

  private:
    bool loginInProgress_ = false;
    Invoke::Domain::Accounts::IAccountManager* account_manager_;
    QNetworkAccessManager* networkManager_;
    QUrl baseUrl_;
    QString current_business_id_;
    void invalidate_business_items();
    QJsonDocument business_items_cache_;
    QElapsedTimer business_items_age_;
    QPointer<QNetworkReply> business_items_reply_;
    quint64 business_items_generation_ = 0;
    quint64 session_generation_ = 0;
    void post_resource(const QString& resource, const QJsonDocument& data,
                       const QJsonDocument& invoice_item = {}, quint64 request_id = 0);
};
} // namespace Infrastructure::Http
