#pragma once

#include <QWidget>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QTabWidget>
#include <QTableWidget>
#include <QPushButton>
#include "Infrastructure/Enums/RouteEnums.h"

namespace App::Views {
class ItemsPage : public QWidget {
    Q_OBJECT
  public:
    explicit ItemsPage(QWidget* parent = nullptr);
    ~ItemsPage() = default;

  public slots:
    void business_selected(const QJsonObject& business);
    void load_items(bool refresh = false);
    void populate_items(const QJsonDocument& items);
    void set_error(const QString& message);

  signals:
    void navigate_to(Page page);
    void items_requested(bool refresh);

  private:
    void create_page_layout();
    void populate_table(QTableWidget* table, const QJsonArray& items, const QStringList& fields);
    QLabel* business_label_;
    QLabel* status_label_;
    QTabWidget* tabs_;
    QTableWidget* business_table_;
    QTableWidget* clients_table_;
    QTableWidget* stock_table_;
    QPushButton* refresh_button_;
    QString business_id_;
};
} // namespace App::Views
