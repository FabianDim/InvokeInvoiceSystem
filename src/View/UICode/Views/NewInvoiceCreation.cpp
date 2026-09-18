#include "View/UICode/Views/NewInvoiceCreation.h"
#include "View/UiStyle.h"
#include <QComboBox>
#include <QDateEdit>
#include <QFileDialog>
#include <QGridLayout>
#include <QJsonObject>
#include <QLineEdit>

using namespace App::Views;

InvoiceDetailsInput::InvoiceDetailsInput(QWidget* parent) : QWidget(parent) {
    create_page_layout();
}

void InvoiceDetailsInput::create_page_layout() {
    auto* content = UiStyle::page_content(this);
    auto* layout = new QVBoxLayout(content);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(16);
    auto* title = UiStyle::label("New invoice", content, "title");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    auto* form = new QGridLayout;
    form->setHorizontalSpacing(16);
    form->setVerticalSpacing(10);
    form->setColumnStretch(1, 1);
    const std::vector<FormField> fields = {
        {"invoice_theme", "Invoice theme", FormField::Type::ComboBox, {}},
        {"invoice_number", "Invoice number", FormField::Type::LineEdit, {.placeholder = "e.g. INV-000123"}},
        {"invoice_file_name", "File name", FormField::Type::LineEdit, {.placeholder = "Name"}},
        {"invoice_file_dir", "Save PDF to", FormField::Type::DirBrowse, {}},
        {"date_created", "Date created", FormField::Type::DateEdit, {.defaultToday = true}},
        {"date_due", "Date due", FormField::Type::DateEdit, {.defaultToday = true}},
        {"website", "Website", FormField::Type::LineEdit, {}}};

    int row = 0;
    for (const auto& field : fields) {
        auto* label = UiStyle::label(field.label, content);
        form->addWidget(label, row, 0);
        QWidget* input = nullptr;
        if (field.type == FormField::Type::LineEdit) {
            auto* edit = new QLineEdit(content);
            edit->setPlaceholderText(field.opt.placeholder);
            input = edit;
        } else if (field.type == FormField::Type::ComboBox) {
            auto* combo = new QComboBox(content);
            combo->addItem("PEECE");
            input = combo;
        } else if (field.type == FormField::Type::DateEdit) {
            auto* date = new QDateEdit(QDate::currentDate(), content);
            date->setCalendarPopup(true);
            date->setDisplayFormat("dd/MM/yyyy");
            input = date;
        } else if (field.type == FormField::Type::DirBrowse) {
            auto* path = new QLineEdit(content);
            path->setReadOnly(true);
            path->setPlaceholderText("Choose a folder");
            auto* browse = new QPushButton("Browse…", content);
            browse->setObjectName("browse_folder_button");
            UiStyle::button(browse);
            auto* folder_row = new QHBoxLayout;
            folder_row->setSpacing(8);
            folder_row->addWidget(path, 1);
            folder_row->addWidget(browse);
            form->addLayout(folder_row, row, 1);
            connect(browse, &QPushButton::clicked, this, [this, path]() {
                const auto directory = QFileDialog::getExistingDirectory(this, "Save invoice to", path->text());
                if (!directory.isEmpty())
                    path->setText(directory);
            });
            input = path;
        }
        if (input) {
            input->setObjectName(field.key);
            input->setProperty("role", "input");
            input->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            label->setBuddy(input);
            base_invoice_form_fields_[field.key] = input;
            if (field.type != FormField::Type::DirBrowse)
                form->addWidget(input, row, 1);
        }
        ++row;
    }
    layout->addLayout(form);

    auto* buttons = new QHBoxLayout;
    buttons->setSpacing(12);
    auto* back = new QPushButton("Back to dashboard", content);
    auto* next = new QPushButton("Next", content);
    UiStyle::button(back);
    UiStyle::button(next, "primary");
    buttons->addWidget(back, 1);
    buttons->addWidget(next, 1);
    layout->addLayout(buttons);
    connect(back, &QPushButton::clicked, this, [this]() { emit invoice_navigation(Page::Dashboard); });
    connect(next, &QPushButton::clicked, this, [this]() {
        const auto text = [this](const QString& key) {
            return qobject_cast<QLineEdit*>(base_invoice_form_fields_.value(key))->text();
        };
        const auto date = [this](const QString& key) {
            return qobject_cast<QDateEdit*>(base_invoice_form_fields_.value(key))->date().toString(Qt::ISODate);
        };
        const auto theme = qobject_cast<QComboBox*>(base_invoice_form_fields_.value("invoice_theme"))->currentText();
        emit set_invoice_details(QJsonDocument(QJsonObject{
            {"invoice_number", text("invoice_number")}, {"invoice_theme", theme},
            {"file_name", text("invoice_file_name")}, {"file_dir", text("invoice_file_dir")},
            {"date_created", date("date_created")}, {"date_due", date("date_due")},
            {"website", text("website")}}));
    });
}
