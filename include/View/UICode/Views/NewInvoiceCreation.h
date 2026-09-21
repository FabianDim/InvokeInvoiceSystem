#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <qmap.h>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include "Utils/FormUtils.h"
#include "Infrastructure/Enums/RouteEnums.h"
#include "Domain/Invoices/Invoice.h"

namespace App::Views {
class InvoiceDetailsInput : public QWidget {
    Q_OBJECT
  public:
    InvoiceDetailsInput(QWidget* parent = nullptr);
    ~InvoiceDetailsInput() = default;
  public slots:
    void reset_form();
    void business_selected(const QJsonObject& business);
    void load_clients(bool refresh = false);
    void populate_clients(const QJsonDocument& items);
    void set_client_error(const QString& message);
  signals:
    void invoice_navigation(Page page);
    void set_invoice_details(const QJsonDocument& doc);
    void clients_requested(bool refresh);

  private:
    void create_page_layout();
    void update_client_selection();
    QComboBox* client_select_;
    QLabel* client_details_;
    QLabel* client_status_;
    QPushButton* refresh_clients_;
    QPushButton* next_button_;
    QString business_id_;

    QMap<QString, QWidget*> base_invoice_form_fields_;
};
} // namespace App::Views
