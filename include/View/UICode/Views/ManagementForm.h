#pragma once
#include "View/UICode/Views/InvoicePreferencesForm.h"
#include "Domain/Records/RecordDefinition.h"

#include <QWidget>
#include <QVector>
#include <QString>
#include <QJsonDocument>
#include "Infrastructure/Enums/RouteEnums.h"

class QFormLayout;
class QLineEdit;
class QLabel;
class QComboBox;
class QCheckBox;

namespace App::Views {
class ManagementForm : public QWidget {
    Q_OBJECT
  public:
    ManagementForm(const QString& title, const QString& resource, const QVector<QString>& fields, QWidget* parent = nullptr,
                   const QJsonObject& record = {}, bool editing = false);
    explicit ManagementForm(const Domain::Records::RecordDefinition& definition, QWidget* parent = nullptr);
    ManagementForm(const Domain::Records::RecordDefinition& definition, const QJsonObject& record, QWidget* parent = nullptr);

  signals:
    void navigate_to(Page page);
    void submit_resource(const QString& resource, const QJsonDocument& data);

  public slots:
    void set_status(const QString& message);
    void set_account_details(const QString& first_name, const QString& last_name, const QString& email);
    void set_offline(bool offline);
    void select_logo(const QString& path);
    void save_succeeded();
    void save_failed(const QString& message);
    bool saving() const { return saving_; }

  protected:
    void showEvent(QShowEvent* event) override;

  private:
    const Domain::Records::RecordDefinition* definition_ = nullptr;
    QJsonObject original_record_;
    bool editing_ = false;
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
    QCheckBox* gst_registered_ = nullptr;
    InvoicePreferencesForm* invoice_preferences_ = nullptr;
    bool saving_ = false;
    bool add_another_ = false;
    void set_saving(bool saving);
    void clear_form();
};
} // namespace App::Views
