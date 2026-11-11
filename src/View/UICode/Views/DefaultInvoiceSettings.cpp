#include "View/UICode/Views/DefaultInvoiceSettings.h"
#include "View/UiStyle.h"
#include <QFileDialog>
#include <QFormLayout>
#include <QLineEdit>

using namespace App::Views;

DefaultInvoiceSettings::DefaultInvoiceSettings(QWidget* parent) : QWidget(parent) {
    auto* content = UiStyle::page_content(this);
    auto* layout = new QVBoxLayout(content);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(16);

    auto* title = UiStyle::label("Default invoice settings", content, "title");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    auto* form = new QFormLayout;
    form->setSpacing(12);
    form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    invoice_prefix_ = new QLineEdit(content);
    invoice_prefix_->setObjectName("default_invoice_prefix");
    invoice_prefix_->setPlaceholderText("e.g. INV-");
    invoice_prefix_->setProperty("role", "input");
    auto* prefix_label = UiStyle::label("Invoice prefix", content);
    prefix_label->setBuddy(invoice_prefix_);
    form->addRow(prefix_label, invoice_prefix_);

    file_out_path_ = new QLineEdit(content);
    file_out_path_->setObjectName("default_invoice_file_out_path");
    file_out_path_->setPlaceholderText("Choose an output folder");
    file_out_path_->setProperty("role", "input");
    auto* path_label = UiStyle::label("File out path", content);
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

    // Form only: persistence and applying these defaults to invoices are intentionally left unconnected.
}

QString DefaultInvoiceSettings::invoice_prefix() const {
    return invoice_prefix_->text();
}

QString DefaultInvoiceSettings::file_out_path() const {
    return file_out_path_->text();
}

void DefaultInvoiceSettings::reset_form() {
    invoice_prefix_->clear();
    file_out_path_->clear();
}
