#include "View/UICode/Views/NewInvoiceStock.h"
#include "Domain/Stock/StockItem.h"
#include "Domain/Invoices/Invoice.h"
#include <QGroupBox>
#include <QJsonDocument>
using namespace App::Views;

NewInvoiceStock::NewInvoiceStock(QWidget* parent) : QWidget(parent) {
    parent_widget_ = new QWidget(this);
    item_form_layout_ = new QWidget(parent_widget_);
    create_page_layout();
}
/*Create the page layout*/
void NewInvoiceStock::create_page_layout() {
    QGroupBox* groupBox = new QGroupBox(tr("Invoice"));
    QVBoxLayout* groupVbox = new QVBoxLayout;
    QGridLayout* main_form_layout = new QGridLayout(this);
    main_form_layout->setAlignment(Qt::AlignCenter);
    main_form_layout->setObjectName("form_grid_layout");

    QWidget* item_list = new QWidget(item_form_layout_);
    auto* hbox = new QHBoxLayout(item_list);
    auto* name_col = new QVBoxLayout();
    auto* qty_col = new QVBoxLayout();
    auto* price_col = new QVBoxLayout();
    auto* name_label = new QLabel("Item Name", item_list);
    auto* qty_label = new QLabel("Quantity", item_list);
    auto* price_label = new QLabel("Price", item_list);
    name_label->setObjectName("form_label");
    qty_label->setObjectName("form_label");
    price_label->setObjectName("form_label");

    name_col->addWidget(name_label);
    qty_col->addWidget(qty_label);
    price_col->addWidget(price_label);
    hbox->addLayout(name_col);
    hbox->addLayout(qty_col);
    hbox->addLayout(price_col);
    hbox->setAlignment(Qt::AlignTop);
    connect(this,
            &NewInvoiceStock::add_item_to_invoice,
            this,
            [name_col, qty_col, price_col, item_list](const QJsonObject& doc) {
                auto* item_label_ = new QLabel(doc.value("Name").toString(), item_list);
                auto* qty_label_ = new QLabel(QString::number(doc.value("Quantity").toInt()), item_list);
                auto* price_label_ = new QLabel(QString::number(doc.value("Price").toDouble(), 'f', 2), item_list);
                name_col->addWidget(item_label_);
                qty_col->addWidget(qty_label_);
                price_col->addWidget(price_label_);
            });

    groupBox->setLayout(groupVbox);
    groupVbox->addWidget(item_list);
    main_form_layout->addWidget(groupBox, 0, 0, 1, 2);
    main_form_layout->addLayout(create_item_entry_form(), 10, 0, 1, 2);

    create_invoice_pdf = new QPushButton("Finish Invoice", item_form_layout_);
    main_form_layout->addWidget(create_invoice_pdf, 20, 0, 1, 2, Qt::AlignCenter);
    std::vector<FormField> fields = {

    };

    for (auto& element : fields) {
        QLabel* label = new QLabel(element.label, item_form_layout_);
        label->setObjectName("form_label");
        int row = main_form_layout->rowCount();
        main_form_layout->addWidget(label, row, 0);
        if (element.type == FormField::Type::LineEdit) {
            QLineEdit* input = new QLineEdit(item_form_layout_);
            input->setObjectName("form_input");
            input->setPlaceholderText(element.opt.placeholder);
            main_form_layout->addWidget(input, row, 1);
            invoice_body_form_fields[element.key] = input;
        } else if (element.type == FormField::Type::ComboBox) {
            QComboBox* input = new QComboBox(item_form_layout_);
            input->setObjectName("form_input");
            main_form_layout->addWidget(input, row, 1);
            invoice_body_form_fields[element.key] = input;
        } else if (element.type == FormField::Type::DateEdit && element.opt.fromNow) {
            QDateEdit* input = new QDateEdit(QDate::currentDate(), item_form_layout_);
            input->setDateRange(QDate::currentDate(), QDate::currentDate().addYears(100));
            input->setObjectName("form_date_input");
            input->setCalendarPopup(true);
            main_form_layout->addWidget(input, row, 1);
            invoice_body_form_fields[element.key] = input;
        } else if (element.type == FormField::Type::DateEdit && element.opt.defaultToday) {
            QDateEdit* input = new QDateEdit(QDate::currentDate(), item_form_layout_);
            input->setObjectName("form_date_input");
            input->setCalendarPopup(true);
            main_form_layout->addWidget(input, row, 1);
            invoice_body_form_fields[element.key] = input;
        } else if (element.type == FormField::Type::DateEdit && !element.opt.defaultToday) {
            QDateEdit* input = new QDateEdit(item_form_layout_);
            input->setObjectName("form_date_input");
            input->setCalendarPopup(true);
            main_form_layout->addWidget(input, row, 1);
            invoice_body_form_fields[element.key] = input;
        }
    }
    this->setLayout(main_form_layout);
}
/*create the item entry and connect buttons to frontend and backend tasks*/
QLayout* App::Views::NewInvoiceStock::create_item_entry_form() {
    auto* row = new QHBoxLayout();
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(8);

    auto* item_name_entry = new QLineEdit();
    auto* item_quantity_entry = new QLineEdit();
    auto* item_price_entry = new QLineEdit();
    auto* item_notes_entry = new QLineEdit();
    auto* add_item_button = new QPushButton("Add Item");

    item_quantity_entry->setValidator(new QIntValidator(1, 1'000'000, item_quantity_entry));
    item_price_entry->setValidator(new QDoubleValidator(0, 1e9, 2, item_price_entry));

    auto addPair = [&](const QString& text, QWidget* edit) {
        auto* pair = new QWidget(this);
        auto* h = new QHBoxLayout(pair);
        h->setContentsMargins(0, 0, 0, 0);
        h->setSpacing(4);
        h->addWidget(new QLabel(text, pair));
        h->addWidget(edit, 1);
        row->addWidget(pair, 1);
    };

    addPair("Item Name:", item_name_entry);
    addPair("Quantity:", item_quantity_entry);
    addPair("Price (each):", item_price_entry);
    addPair("Notes:", item_notes_entry);

    row->addWidget(add_item_button);

    /*Create a JSon object and add it to a JSon document*/
    connect(add_item_button, &QPushButton::clicked, this, [=, this]() {
        QJsonObject item;
        item["Quantity"] = (item_quantity_entry->text().toInt());
        item["Price"] = (item_price_entry->text().toFloat());
        item["Name"] = (item_name_entry->text());
        item["Notes"] = (item_notes_entry->text());

        item_name_entry->clear();
        item_quantity_entry->clear();
        item_price_entry->clear();
        item_notes_entry->clear();
        stock_items.append(item);
        emit add_item_to_invoice(item);
    });

    /*send the json document to the app controller to send to the backend*/
    const QJsonDocument json = QJsonDocument(stock_items);
    connect(
        create_invoice_pdf, &QPushButton::clicked, this, [json, this](bool) { emit add_item_list_to_invoice(json); });

    return row;
}
