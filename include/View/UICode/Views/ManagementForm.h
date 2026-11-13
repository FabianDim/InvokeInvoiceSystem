#pragma once

#include <QWidget>
#include <QVector>
#include <QString>
#include <QJsonDocument>
#include "Infrastructure/Enums/RouteEnums.h"

class QFormLayout;
class QLineEdit;
class QLabel;
class QComboBox;

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
    void save_succeeded();
    void save_failed(const QString& message);
    bool saving() const { return saving_; }

  protected:
    void showEvent(QShowEvent* event) override;

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
    QComboBox* country_ = nullptr;
    bool saving_ = false;
    bool add_another_ = false;
    void set_saving(bool saving);
    void clear_form();
};
} // namespace App::Views
