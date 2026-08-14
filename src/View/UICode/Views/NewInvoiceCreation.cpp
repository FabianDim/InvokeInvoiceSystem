#include "View/UICode/Views/NewInvoiceCreation.h"
#include <QLineEdit>
#include <qlabel.h>
#include <QComboBox>
#include <string>
#include "pch.h"
#include <QDateEdit>
#include <QPushButton>
#include "Infrastructure/Enums/RouteEnums.h"
#include "Domain/Invoices/InvoiceTemplateEnum.h"
#include "Domain/Invoices/Invoice.h"
#include <QJsonObject>
#include <qfiledialog.h>
using namespace App::Views;
InvoiceDetailsInput::InvoiceDetailsInput(QWidget* parent) {
    parent_widget_ = new QWidget(this);
    form_layout_ = new QWidget(parent_widget_);
    create_page_layout();
}

void InvoiceDetailsInput::create_page_layout() {
    QGridLayout* main_form_layout = new QGridLayout(form_layout_);
    main_form_layout->setAlignment(Qt::AlignCenter);
    main_form_layout->setObjectName("form_grid_layout");
    std::vector<FormField> fields = {
        {"invoice_theme", "Invoice Theme:", FormField::Type::ComboBox, {}},
        {"invoice_number", "Invoice Number:", FormField::Type::LineEdit, {.placeholder = "e.g. INV-000123"}},
        {"invoice_file_name", "File Name:", FormField::Type::LineEdit, {.placeholder = "Name"}},
        {"invoice_file_dir", "Save PDF to:", FormField::Type::DirBrowse, {}},
        {"date_created", "Date Created:", FormField::Type::DateEdit, {.defaultToday = true}},
        {"date_due", "Date Due:", FormField::Type::DateEdit, {.defaultToday = true}},
        {"website", "Website", FormField::Type::LineEdit, {}}};
    QLineEdit* output_path_edit = new QLineEdit(this);
    output_path_edit->setReadOnly(true);
    output_path_edit->setHidden(true);
    for (auto& element : fields) {
        QLabel* label = new QLabel(element.label, form_layout_);
        label->setObjectName("form_label");
        int row = main_form_layout->rowCount();
        main_form_layout->addWidget(label, row, 0);
        if (element.type == FormField::Type::LineEdit) {
            QLineEdit* input = new QLineEdit(form_layout_);
            input->setObjectName("form_input");
            input->setPlaceholderText(element.opt.placeholder);
            main_form_layout->addWidget(input, row, 1);
            base_invoice_form_fields_[element.key] = input;
        } else if (element.type == FormField::Type::ComboBox) {
            QComboBox* input = new QComboBox(form_layout_);
            input->setObjectName("form_input");
            main_form_layout->addWidget(input, row, 1);
            base_invoice_form_fields_[element.key] = input;
        } else if (element.type == FormField::Type::DateEdit && element.opt.fromNow) {
            QDateEdit* input = new QDateEdit(QDate::currentDate(), form_layout_);
            input->setDateRange(QDate::currentDate(), QDate::currentDate().addYears(100));
            input->setObjectName("form_date_input");
            input->setCalendarPopup(true);
            main_form_layout->addWidget(input, row, 1);
            base_invoice_form_fields_[element.key] = input;
        } else if (element.type == FormField::Type::DateEdit && element.opt.defaultToday) {
            QDateEdit* input = new QDateEdit(QDate::currentDate(), form_layout_);
            input->setObjectName("form_date_input");
            input->setCalendarPopup(true);
            main_form_layout->addWidget(input, row, 1);
            base_invoice_form_fields_[element.key] = input;
        } else if (element.type == FormField::Type::DateEdit && !element.opt.defaultToday) {
            QDateEdit* input = new QDateEdit(form_layout_);
            input->setObjectName("form_date_input");
            input->setCalendarPopup(true);
            main_form_layout->addWidget(input, row, 1);
            base_invoice_form_fields_[element.key] = input;
        } else if (element.type == FormField::Type::DirBrowse) {
            dir_button_ = new QPushButton("Browse...", form_layout_);
            dir_button_->setObjectName("form_dir_input");
            main_form_layout->addWidget(dir_button_, row, 1);
            connect(dir_button_, &QPushButton::clicked, this, [this, output_path_edit, element]() {
                QString file = QFileDialog::getExistingDirectory();
                if (!file.isEmpty()) {
                    output_path_edit->setText(file);
                }
                base_invoice_form_fields_[element.key] = output_path_edit;
            });
        }
    }

    form_layout_->setLayout(main_form_layout);
    this->setLayout(main_form_layout);

    QPushButton* next_page_button = new QPushButton("Next >>", parent_widget_);

    connect(next_page_button, &QPushButton::clicked, this, [this]() {
        auto idLe = qobject_cast<QLineEdit*>(base_invoice_form_fields_.value("invoice_number"));
        auto theme = qobject_cast<QComboBox*>(base_invoice_form_fields_.value("invoice_theme"));
        auto file_name = qobject_cast<QLineEdit*>(base_invoice_form_fields_.value("invoice_file_name"));
        auto created = qobject_cast<QDateEdit*>(base_invoice_form_fields_.value("date_created"));
        auto due = qobject_cast<QDateEdit*>(base_invoice_form_fields_.value("date_due"));
        auto dir_name = qobject_cast<QLineEdit*>(base_invoice_form_fields_.value("invoice_file_dir"));
        auto website = qobject_cast<QLineEdit*>(base_invoice_form_fields_.value("website"));
        if (!idLe || !theme || !created || !due || !file_name || !dir_name) {
            return;
        }

        QJsonObject inv_obj = {{"invoice_number", idLe->text()},
                               {"invoice_theme", theme->currentText()},
                               {"file_name", file_name->text()},
                               {"date_created", created->date().toString(Qt::ISODate)},
                               {"date_due", due->date().toString(Qt::ISODate)},
                               {"file_dir", dir_name->text()},
                               {"website", website->text()}};

        QJsonDocument doc(inv_obj);
        emit set_invoice_details(doc);

        emit invoice_navigation(Page::StockInput);
    });

    main_form_layout->addWidget(next_page_button, main_form_layout->rowCount(), 1, Qt::AlignRight);
}

void InvoiceDetailsInput::create_form_layout() {
    QGridLayout* main_form_layout = new QGridLayout(form_layout_);
    main_form_layout->setAlignment(Qt::AlignCenter);

    QLabel* invoice_label = new QLabel("Invoice Number:", form_layout_);
    QLineEdit* invoice_input = new QLineEdit(form_layout_);
    invoice_label->setObjectName("form_label");
    invoice_input->setObjectName("form_input");

    main_form_layout->addWidget(invoice_label, 0, 0);
    main_form_layout->addWidget(invoice_input, 0, 1);

    QLabel* business_label = new QLabel("Choose your business:", form_layout_);
    QComboBox* business_select = new QComboBox(form_layout_);
    business_label->setObjectName("form_label");
    business_select->setObjectName("form_input");

    main_form_layout->addWidget(business_label, 1, 0);
    main_form_layout->addWidget(business_select, 1, 1);

    QLabel* client_name_label = new QLabel("Client Name:", form_layout_);
    QComboBox* client_select = new QComboBox(form_layout_);
    client_name_label->setObjectName("form_label");

    main_form_layout->addWidget(client_name_label, 2, 0);
    main_form_layout->addWidget(client_select, 2, 1);

    form_layout_->setLayout(main_form_layout);
    this->setLayout(main_form_layout);
}