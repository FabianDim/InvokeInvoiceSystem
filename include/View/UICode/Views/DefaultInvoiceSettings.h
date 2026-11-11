#pragma once

#include <QWidget>
#include "Infrastructure/Enums/RouteEnums.h"

class QLineEdit;

namespace App::Views {
class DefaultInvoiceSettings : public QWidget {
    Q_OBJECT
  public:
    explicit DefaultInvoiceSettings(QWidget* parent = nullptr);
    QString invoice_prefix() const;
    QString file_out_path() const;
    void reset_form();

  signals:
    void navigate_to(Page page);

  private:
    QLineEdit* invoice_prefix_;
    QLineEdit* file_out_path_;
};
} // namespace App::Views
