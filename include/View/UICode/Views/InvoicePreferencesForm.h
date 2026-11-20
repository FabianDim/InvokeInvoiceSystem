#pragma once
#include "Domain/Business/InvoicePreferences.h"
#include "View/UiStyle.h"
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QLineEdit>

namespace App::Views {
class InvoicePreferencesForm : public QWidget {
  public:
    explicit InvoicePreferencesForm(QWidget* parent, bool include_gst = true) : QWidget(parent) {
        auto* form = new QFormLayout(this);
        form->setContentsMargins(0, 0, 0, 0);
        form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
        gst_ = new QCheckBox("Registered for GST (adds 10% to invoice items)", this);
        gst_->setObjectName("settings_gst_registered");
        if (include_gst) form->addRow(gst_);
        else gst_->hide();
        terms_ = new QComboBox(this);
        terms_->setObjectName("business_payment_terms");
        terms_->setProperty("role", "input");
        terms_->addItem("COD (due on invoice date)", "COD");
        terms_->addItem("14 days", "14");
        terms_->addItem("30 days", "30");
        auto* terms_label = UiStyle::label("Payment terms", this);
        terms_label->setBuddy(terms_);
        form->addRow(terms_label, terms_);
        const QList<QPair<QString, QString>> fields{{"BankName", "Bank name"},
            {"BankAccountName", "Account name"}, {"BankBSB", "BSB"}, {"BankAccountNumber", "Account number"}};
        for (const auto& field : fields) {
            auto* input = new QLineEdit(this);
            input->setObjectName(field.first);
            input->setProperty("role", "input");
            input->setMaxLength(120);
            input->setPlaceholderText(field.second + " (optional)");
            auto* label = UiStyle::label(field.second + " (optional)", this);
            label->setBuddy(input);
            form->addRow(label, input);
            bank_[field.first] = input;
        }
    }
    QJsonObject values() const {
        QJsonObject data{{"GSTRegistered", gst_->isChecked()}, {"PaymentTerms", terms_->currentData().toString()}};
        for (auto it = bank_.begin(); it != bank_.end(); ++it) data[it.key()] = it.value()->text().trimmed();
        return data;
    }
    void set_values(const QJsonObject& data) {
        gst_->setChecked(Domain::Business::gst_registered(data));
        const auto index = terms_->findData(data.value("PaymentTerms").toString("COD"));
        terms_->setCurrentIndex(index < 0 ? 0 : index);
        for (auto it = bank_.begin(); it != bank_.end(); ++it) it.value()->setText(data.value(it.key()).toString());
    }
  private:
    QCheckBox* gst_;
    QComboBox* terms_;
    QMap<QString, QLineEdit*> bank_;
};
}
