#include "View/UICode/Views/DefaultInvoiceSettings.h"
#include "View/UiStyle.h"
#include <QFileDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QSettings>


using namespace App::Views;

DefaultInvoiceSettings::DefaultInvoiceSettings(QWidget* parent) : QWidget(parent) {
    auto* content = UiStyle::page_content(this);
    auto* layout = new QVBoxLayout(content);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(16);

    auto* title = UiStyle::label("Business invoice settings", content, "title");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);
    auto* business_label = UiStyle::label("", content, "subtitle");
    business_label->setObjectName("invoice_settings_business");
    business_label->setTextFormat(Qt::PlainText);
    layout->addWidget(business_label);

    auto* form = new QFormLayout;
    form->setSpacing(12);
    form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    invoice_prefix_ = new QLineEdit(content);
    invoice_prefix_->setObjectName("default_invoice_prefix");
    invoice_prefix_->setMaxLength(40);
    invoice_prefix_->setPlaceholderText("e.g. INV-");
    invoice_prefix_->setProperty("role", "input");
    auto* prefix_label = UiStyle::label("Invoice prefix (saved with this business)", content);
    prefix_label->setBuddy(invoice_prefix_);
    form->addRow(prefix_label, invoice_prefix_);

    file_out_path_ = new QLineEdit(content);
    file_out_path_->setObjectName("default_invoice_file_out_path");
    file_out_path_->setPlaceholderText("Choose an output folder");
    file_out_path_->setProperty("role", "input");
    auto* path_label = UiStyle::label("Default PDF folder on this computer", content);
    path_label->setBuddy(file_out_path_);
    auto* path_row = new QHBoxLayout;
    path_row->addWidget(file_out_path_, 1);
    auto* browse = new QPushButton("Browse...", content);
    browse->setObjectName("default_invoice_browse");
    UiStyle::button(browse);
    path_row->addWidget(browse);
    form->addRow(path_label, path_row);
    layout->addLayout(form);

    auto* back = new QPushButton("Back to dashboard", content);
    back->setObjectName("default_invoice_back");
    UiStyle::button(back);
    layout->addWidget(back);
    connect(back, &QPushButton::clicked, this, [this]() { emit navigate_to(Page::Dashboard); });
    connect(browse, &QPushButton::clicked, this, [this]() {
        const auto path = QFileDialog::getExistingDirectory(this, "Choose invoice output folder", file_out_path_->text());
        if (!path.isEmpty())
            file_out_path_->setText(path);
    });

    auto* help = UiStyle::label("Leave the prefix blank for numbers only. Numbers advance automatically for this business.", content);
    help->setWordWrap(true);
    layout->addWidget(help);
    save_ = new QPushButton("Save business invoice settings", content);
    save_->setObjectName("save_invoice_defaults");
    UiStyle::button(save_, "primary");
    layout->addWidget(save_);
    status_ = UiStyle::label("", content, "status");
    status_->setObjectName("invoice_defaults_status");
    layout->addWidget(status_);
    connect(save_, &QPushButton::clicked, this, [this]() {
        if (business_id_.isEmpty()) return;
        for (auto* button : findChildren<QPushButton*>()) button->setEnabled(false);
        invoice_prefix_->setEnabled(false);
        file_out_path_->setEnabled(false);
        status_->setText("Saving business invoice settings...");
        emit save_requested(QJsonObject{{"BusinessID", business_id_}, {"InvoicePrefix", invoice_prefix_->text()}});
    });
}

QString DefaultInvoiceSettings::invoice_prefix() const {
    return saved_prefix_;
}

QString DefaultInvoiceSettings::file_out_path() const {
    return saved_folder_;
}

void DefaultInvoiceSettings::reset_form() {
    business_id_.clear();
    saved_prefix_.clear();
    saved_folder_.clear();
    invoice_prefix_->clear();
    file_out_path_->clear();
    status_->clear();
    for (auto* button : findChildren<QPushButton*>()) button->setEnabled(true);
    save_->setEnabled(!business_id_.isEmpty());
}

void DefaultInvoiceSettings::set_business(const QJsonObject& business, bool offline) {
    findChild<QLabel*>("invoice_settings_business")->setText(business.value("BusinessName").toString());
    const auto id = business.value("_id").toString();
    const bool changed = id != business_id_ || offline != offline_;
    business_id_ = id;
    offline_ = offline;
    invoice_prefix_->setText(business.value("InvoicePrefix").toString("INV-"));
    if (changed) {
        QSettings settings(QSettings::defaultFormat(), QSettings::UserScope, "Invoke", "InvokeInvoiceSystem");
        file_out_path_->setText(offline ? QString{} : settings.value("businesses/" + id + "/pdfFolder").toString());
    }
    saved_prefix_ = invoice_prefix_->text();
    saved_folder_ = file_out_path_->text();
    save_->setEnabled(!id.isEmpty());
    invoice_prefix_->setEnabled(true);
    file_out_path_->setEnabled(true);
    status_->clear();
}
void DefaultInvoiceSettings::save_succeeded() {
    saved_prefix_ = invoice_prefix_->text();
    saved_folder_ = file_out_path_->text();
    if (!offline_) {
        QSettings settings(QSettings::defaultFormat(), QSettings::UserScope, "Invoke", "InvokeInvoiceSystem");
        settings.setValue("businesses/" + business_id_ + "/pdfFolder", file_out_path_->text());
        settings.sync();
        if (settings.status() != QSettings::NoError) {
            save_failed("Business prefix saved, but the PDF folder could not be saved on this computer.");
            return;
        }
    }
    for (auto* button : findChildren<QPushButton*>()) button->setEnabled(true);
    invoice_prefix_->setEnabled(true);
    file_out_path_->setEnabled(true);
    status_->setText("Business invoice settings saved.");
}
void DefaultInvoiceSettings::save_failed(const QString& message) {
    for (auto* button : findChildren<QPushButton*>()) button->setEnabled(true);
    save_->setEnabled(!business_id_.isEmpty());
    invoice_prefix_->setEnabled(true);
    file_out_path_->setEnabled(true);
    status_->setText(message);
}
