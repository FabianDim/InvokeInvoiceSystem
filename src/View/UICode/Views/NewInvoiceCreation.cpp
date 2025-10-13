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
using namespace App::Views;
InvoiceDetailsInput::InvoiceDetailsInput(Invoice* invoice, QWidget* parent) {
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
        {"date_created", "Date Created:", FormField::Type::DateEdit, {.defaultToday = true}},
        {"date_due", "Date Due:", FormField::Type::DateEdit, {.defaultToday = false}},
    };

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
        }
    }

    form_layout_->setLayout(main_form_layout);
    this->setLayout(main_form_layout);

    QPushButton* next_page_button = new QPushButton("Next >>", parent_widget_);

    connect(next_page_button, &QPushButton::clicked, this, [this]() {
        auto idLe = qobject_cast<QLineEdit*>(base_invoice_form_fields_.value("invoice_number"));
        auto theme = qobject_cast<QComboBox*>(base_invoice_form_fields_.value("invoice_theme"));
        auto created = qobject_cast<QDateEdit*>(base_invoice_form_fields_.value("date_created"));
        auto due = qobject_cast<QDateEdit*>(base_invoice_form_fields_.value("date_due"));

        if (!idLe || !theme || !created || !due) {
            return;
        }

        invoice->setInvoiceID(idLe->text().toStdString());
        invoice->setTemplate(theme->currentText() == "PEECE" ? InvoiceTemplateEnum::PEECE : InvoiceTemplateEnum::PEECE);

        // Prefer dates, not text, then format:
        invoice->setCurrentDate(created->date().toString(Qt::ISODate).toStdString());
        invoice->setDueDate(due->date().toString(Qt::ISODate).toStdString());

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