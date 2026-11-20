#pragma once

#include <QWidget>
#include <QPointer>
#include <QDialog>
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
    void stock_updated(quint64 request_id);
    void stock_update_failed(const QString& message, quint64 request_id);

  signals:
    void navigate_to(Page page);
    void items_requested(bool refresh);
    void stock_update_requested(const QJsonObject& data, quint64 request_id);

  private:
    void create_page_layout();
    void edit_stock(int row);
    void update_edit_button();
    QPointer<QDialog> stock_editor_;
    quint64 stock_edit_request_ = 0;
    QPushButton* edit_stock_button_;
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
