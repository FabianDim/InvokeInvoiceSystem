#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <qmap.h>

#include "Utils/FormUtils.h"
#include "Infrastructure/Enums/RouteEnums.h"
#include "Domain/Invoices/Invoice.h"

namespace App::Views {
class InvoiceDetailsInput : public QWidget {
    Q_OBJECT
  public:
    InvoiceDetailsInput(QWidget* parent = nullptr);
    ~InvoiceDetailsInput() = default;
  signals:
    void invoice_navigation(Page page);
    void set_invoice_details(const QJsonDocument& doc);

  private:
    void create_page_layout();
    void create_form_layout();

    QMap<QString, QWidget*> base_invoice_form_fields_;

    QWidget* parent_widget_;
    QWidget* form_layout_;
};
} // namespace App::Views