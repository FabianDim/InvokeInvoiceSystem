#pragma once

#include <QWidget>
#include <QVector>
#include <QString>
#include "Infrastructure/Enums/RouteEnums.h"

class QFormLayout;
class QLineEdit;
class QLabel;

namespace App::Views {
class ManagementForm : public QWidget {
    Q_OBJECT
  public:
    ManagementForm(const QString& title, const QVector<QString>& fields, QWidget* parent = nullptr);

  signals:
    void navigate_to(Page page);

  private:
    void submit_form();
    QFormLayout* form_layout_ = nullptr;
    QLabel* status_label_ = nullptr;
    QVector<QLineEdit*> inputs_;
};
} // namespace App::Views
