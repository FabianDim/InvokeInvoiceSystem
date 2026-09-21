#pragma once

#include <QWidget>
#include <QVector>
#include <QString>
#include <QJsonDocument>
#include "Infrastructure/Enums/RouteEnums.h"

class QFormLayout;
class QLineEdit;
class QLabel;

namespace App::Views {
class ManagementForm : public QWidget {
    Q_OBJECT
  public:
    ManagementForm(const QString& title, const QString& resource, const QVector<QString>& fields, QWidget* parent = nullptr);

  signals:
    void navigate_to(Page page);
    void submit_resource(const QString& resource, const QJsonDocument& data);

  public slots:
    void set_status(const QString& message);
    void set_offline(bool offline);
    void select_logo(const QString& path);

  private:
    bool field_required(const QString& field) const;
    void submit_form();
    QFormLayout* form_layout_ = nullptr;
    QLabel* status_label_ = nullptr;
    QVector<QLineEdit*> inputs_;
    QVector<QString> field_names_;
    QString resource_;
    QLineEdit* logo_path_ = nullptr;
    QByteArray logo_data_;
};
} // namespace App::Views
