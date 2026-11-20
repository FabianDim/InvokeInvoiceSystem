#include "View/UICode/Views/DefaultInvoiceSettings.h"
#include "View/UiStyle.h"
#include <QFileDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QSettings>
#include "Domain/Invoices/InvoiceFilename.h"


using namespace App::Views;

DefaultInvoiceSettings::DefaultInvoiceSettings(QWidget* parent) : QWidget(parent) {
    auto* content = UiStyle::page_content(this);
    auto* layout = new QVBoxLayout(content);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(16);

    auto* title = UiStyle::label("Business invoice settings", content, "title");
    title->setAlignment(Qt::AlignLeft);
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

    auto* filename_label = UiStyle::label("Automatic PDF filename", content);
    filename_template_ = new FilenameTemplateEdit(content);
    filename_label->setBuddy(filename_template_);
    form->addRow(filename_label);
    form->addRow(filename_template_);
    auto* pills = new QHBoxLayout;
    pills->setSpacing(8);
    pills->addWidget(new FilenameTokenPill("Business name", "business_name", filename_template_, content));
    pills->addWidget(new FilenameTokenPill("Invoice number", "invoice_number", filename_template_, content));
    pills->addWidget(new FilenameTokenPill("Invoice number only", "invoice_number_only", filename_template_, content));
    pills->addStretch();
    form->addRow(pills);
    auto* filename_help = UiStyle::label("Drag or click a pill to insert it. Type text and separators such as _ between pills. Invoice number only omits the prefix (000001). Spaces become underscores; .pdf is added automatically. Leave blank to use the invoice number.", content);
    filename_help->setWordWrap(true);
    form->addRow(filename_help);
    filename_preview_ = UiStyle::label("", content);
    filename_preview_->setObjectName("invoice_filename_preview");
    filename_preview_->setTextFormat(Qt::PlainText);
    filename_preview_->setWordWrap(true);
    form->addRow(filename_preview_);
    connect(filename_template_, &QTextEdit::textChanged, this, &DefaultInvoiceSettings::update_filename_preview);
    connect(invoice_prefix_, &QLineEdit::textChanged, this, &DefaultInvoiceSettings::update_filename_preview);
    filename_template_->set_pattern(Domain::Invoices::default_filename_template());

    preferences_ = new InvoicePreferencesForm(content);
    form->addRow(preferences_);
    invoice_terms_ = new QPlainTextEdit(content);
    invoice_terms_->setObjectName("default_invoice_terms");
    invoice_terms_->setPlaceholderText("e.g. Please quote your invoice number when paying. Thank you for your business.");
    invoice_terms_->setProperty("role", "input");
    invoice_terms_->setMinimumHeight(110);
    invoice_terms_->setTabChangesFocus(true);
    auto* terms_label = UiStyle::label("Terms and conditions (optional)", content);
    terms_label->setBuddy(invoice_terms_);
    form->addRow(terms_label, invoice_terms_);
    auto* terms_help = UiStyle::label("Printed at the end of this business's invoices. Leave blank to omit. Maximum 4,000 characters.", content);
    terms_help->setWordWrap(true);
    form->addRow(QString{}, terms_help);

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
        const auto filename_error = Domain::Invoices::filename_template_error(filename_template_->pattern());
        if (!filename_error.isEmpty()) {
            status_->setText(filename_error);
            filename_template_->setFocus();
            return;
        }
        if (invoice_terms_->toPlainText().size() > 4000) {
            status_->setText("Terms and conditions must be at most 4,000 characters.");
            invoice_terms_->setFocus();
            return;
        }
        for (auto* button : findChildren<QPushButton*>()) button->setEnabled(false);
        invoice_prefix_->setEnabled(false);
        filename_template_->setEnabled(false);
        invoice_terms_->setEnabled(false);
        preferences_->setEnabled(false);
        file_out_path_->setEnabled(false);
        status_->setText("Saving business invoice settings...");
        auto settings = preferences_->values();
        settings["BusinessID"] = business_id_;
        settings["InvoicePrefix"] = invoice_prefix_->text();
        settings["InvoiceTerms"] = invoice_terms_->toPlainText();
        settings["InvoiceFilenameTemplate"] = filename_template_->pattern();
        emit save_requested(settings);
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
    business_name_.clear();
    filename_template_->setEnabled(true);
    filename_template_->set_pattern(Domain::Invoices::default_filename_template());
    saved_prefix_.clear();
    saved_folder_.clear();
    invoice_prefix_->clear();
    invoice_terms_->clear();
    preferences_->set_values({});
    invoice_terms_->setEnabled(true);
    preferences_->setEnabled(true);
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
    business_name_ = business.value("BusinessName").toString();
    filename_template_->set_pattern(business.value("InvoiceFilenameTemplate").toString(Domain::Invoices::default_filename_template()));
    invoice_prefix_->setText(business.value("InvoicePrefix").toString("INV-"));
    invoice_terms_->setPlainText(business.value("InvoiceTerms").toString());
    preferences_->set_values(business);
    if (changed) {
        QSettings settings(QSettings::defaultFormat(), QSettings::UserScope, "Invoke", "InvokeInvoiceSystem");
        file_out_path_->setText(offline ? QString{} : settings.value("businesses/" + id + "/pdfFolder").toString());
    }
    saved_prefix_ = invoice_prefix_->text();
    saved_folder_ = file_out_path_->text();
    save_->setEnabled(!id.isEmpty());
    invoice_prefix_->setEnabled(true);
    filename_template_->setEnabled(true);
    invoice_terms_->setEnabled(true);
    preferences_->setEnabled(true);
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
            save_failed("Business invoice settings saved, but the PDF folder could not be saved on this computer.");
            return;
        }
    }
    for (auto* button : findChildren<QPushButton*>()) button->setEnabled(true);
    invoice_prefix_->setEnabled(true);
    filename_template_->setEnabled(true);
    invoice_terms_->setEnabled(true);
    preferences_->setEnabled(true);
    file_out_path_->setEnabled(true);
    status_->setText("Business invoice settings saved.");
}
void DefaultInvoiceSettings::save_failed(const QString& message) {
    for (auto* button : findChildren<QPushButton*>()) button->setEnabled(true);
    save_->setEnabled(!business_id_.isEmpty());
    invoice_prefix_->setEnabled(true);
    filename_template_->setEnabled(true);
    invoice_terms_->setEnabled(true);
    preferences_->setEnabled(true);
    file_out_path_->setEnabled(true);
    status_->setText(message);
}

void DefaultInvoiceSettings::update_filename_preview() {
    const auto pattern = filename_template_->pattern();
    const auto error = Domain::Invoices::filename_template_error(pattern);
    filename_preview_->setText(error.isEmpty()
        ? "Example: " + Domain::Invoices::invoice_filename(pattern, business_name_.isEmpty() ? "Your Business" : business_name_,
                                                          invoice_prefix_->text() + "000001", invoice_prefix_->text()) + ".pdf"
        : error);
}
