#pragma once

#include <QWidget>
#include <QJsonObject>
class QLabel;
class QPushButton;
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
    void set_business(const QJsonObject& business, bool offline = false);
    void save_succeeded();
    void save_failed(const QString& message);

  signals:
    void navigate_to(Page page);
    void save_requested(const QJsonObject& settings);

  private:
    QString business_id_;
    QString saved_prefix_ = "INV-";
    QString saved_folder_;
    bool offline_ = false;
    QLabel* status_;
    QPushButton* save_;
    QLineEdit* invoice_prefix_;
    QLineEdit* file_out_path_;
};
} // namespace App::Views
