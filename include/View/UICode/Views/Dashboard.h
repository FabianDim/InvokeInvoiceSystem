#pragma once
#include "pch.h"
#include <QWidget>
#include <QLabel>
#include <QButtonGroup>
#include <QComboBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <qlayout.h>
#include "Infrastructure/Enums/RouteEnums.h"
#include <qpushbutton.h>

namespace App {
namespace Views {
class Dashboard : public QWidget {
    Q_OBJECT
  public:
    Dashboard(QWidget* parent = nullptr);
    ~Dashboard() = default;
    bool has_business() const;
    void set_offline(bool offline);
    void set_logged_in(bool logged_in);

  public slots:
    void select_after_loading(const QString& id) { pending_business_id_ = id; }
    void set_loading(bool loading);
    void set_error(const QString& message);
    void populate_business_list(const QJsonDocument& list);

  private:
    QString pending_business_id_;
    bool offline_ = false;
    bool logged_in_ = false;
    void create_page_layout();
    QButtonGroup* button_group_;
    void button_connections();
    void update_business_selection();
    QComboBox* business_select_;
    QLabel* loading_label_;
    QPushButton* invoice_button;
    QPushButton* client_button;
    QPushButton* business_button;
    QPushButton* stock_button;
    QPushButton* account_button;
    QPushButton* items_button;
    QPushButton* default_invoice_settings_button;

  signals:
    void dash_navigation(Page page);
    void business_chosen(const QJsonObject& business);
};
} // namespace Views
} // namespace App
