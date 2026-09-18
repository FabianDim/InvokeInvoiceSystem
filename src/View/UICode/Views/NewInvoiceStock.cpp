#include "View/UICode/Views/NewInvoiceStock.h"
#include "View/UiStyle.h"
#include <QGroupBox>
#include <QJsonDocument>
#include <QScrollArea>

using namespace App::Views;

NewInvoiceStock::NewInvoiceStock(QWidget* parent) : QWidget(parent) {
    create_page_layout();
}

void NewInvoiceStock::reset_invoice() {
    stock_items = QJsonArray{};
    for (auto* label : item_labels_)
        delete label;
    item_labels_.clear();
    for (auto* field : invoice_body_form_fields) {
        if (auto* selector = qobject_cast<QComboBox*>(field))
            selector->clear();
        else if (auto* input = qobject_cast<QLineEdit*>(field))
            input->clear();
    }
    create_invoice_pdf->setEnabled(true);
}

void NewInvoiceStock::populate_stock_list(const QJsonDocument& list) {
    auto* selector = qobject_cast<QComboBox*>(invoice_body_form_fields.value("stock_selector"));
    if (!selector || !list.isArray()) return;
    selector->clear();
    for (const auto& value : list.array()) {
        const auto item = value.toObject();
        selector->addItem(item.value("Name").toString(), item);
    }
}

void NewInvoiceStock::create_page_layout() {
    auto* content = UiStyle::page_content(this);
    auto* layout = new QVBoxLayout(content);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(16);
    auto* title = UiStyle::label("Invoice items", content, "title");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    auto* group = new QGroupBox("Items added", content);
    auto* group_layout = new QVBoxLayout(group);
    auto* scroll = new QScrollArea(group);
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setMinimumHeight(140);
    scroll->setMaximumHeight(200);
    auto* item_list = new QWidget(scroll);
    auto* columns = new QHBoxLayout(item_list);
    auto* names = new QVBoxLayout;
    auto* quantities = new QVBoxLayout;
    auto* prices = new QVBoxLayout;
    names->addWidget(UiStyle::label("Item name", item_list));
    quantities->addWidget(UiStyle::label("Quantity", item_list));
    prices->addWidget(UiStyle::label("Price", item_list));
    for (auto* column : {names, quantities, prices}) {
        column->setAlignment(Qt::AlignTop);
        column->setSpacing(10);
    }
    columns->addLayout(names, 2);
    columns->addLayout(quantities, 1);
    columns->addLayout(prices, 1);
    scroll->setWidget(item_list);
    group_layout->addWidget(scroll);
    layout->addWidget(group);
    connect(this, &NewInvoiceStock::add_item_to_invoice, this,
            [this, names, quantities, prices, item_list](const QJsonObject& item) {
                auto* name = UiStyle::label(item.value("Name").toString(), item_list, "body");
                auto* quantity = UiStyle::label(QString::number(item.value("Quantity").toInt()), item_list, "body");
                auto* price = UiStyle::label(QString::number(item.value("Price").toDouble(), 'f', 2), item_list, "body");
                names->addWidget(name);
                quantities->addWidget(quantity);
                prices->addWidget(price);
                item_labels_ << name << quantity << price;
            });

    create_invoice_pdf = new QPushButton("Finish invoice", content);
    auto* back = new QPushButton("Back to dashboard", content);
    UiStyle::button(create_invoice_pdf, "primary");
    UiStyle::button(back);
    layout->addLayout(create_item_entry_form());
    auto* buttons = new QHBoxLayout;
    buttons->setSpacing(12);
    buttons->addWidget(back, 1);
    buttons->addWidget(create_invoice_pdf, 1);
    layout->addLayout(buttons);
    connect(back, &QPushButton::clicked, this, [this]() { emit invoice_navigation(Page::Dashboard); });
}

QLayout* NewInvoiceStock::create_item_entry_form() {
    auto* form = new QGridLayout;
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(8);
    form->setColumnStretch(0, 1);
    form->setColumnStretch(1, 1);
    auto* name = new QComboBox(this);
    name->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    auto* quantity = new QLineEdit(this);
    auto* price = new QLineEdit(this);
    auto* notes = new QLineEdit(this);
    auto* add = new QPushButton("Add item", this);
    UiStyle::button(add);
    quantity->setValidator(new QIntValidator(1, 1'000'000, quantity));
    price->setValidator(new QDoubleValidator(0, 1e9, 2, price));
    quantity->setPlaceholderText("Quantity");
    price->setPlaceholderText("Use stock price");
    notes->setPlaceholderText("Optional notes");
    const auto add_field = [this, form](const QString& text, QWidget* edit, int row, int column) {
        auto* label = UiStyle::label(text, this);
        label->setBuddy(edit);
        edit->setProperty("role", "input");
        edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        form->addWidget(label, row, column);
        form->addWidget(edit, row + 1, column);
    };
    add_field("Stock item", name, 0, 0);
    add_field("Quantity", quantity, 0, 1);
    add_field("Price each", price, 2, 0);
    add_field("Notes", notes, 2, 1);
    form->addWidget(add, 4, 1);
    invoice_body_form_fields["stock_selector"] = name;
    invoice_body_form_fields["quantity"] = quantity;
    invoice_body_form_fields["price"] = price;
    invoice_body_form_fields["notes"] = notes;

    connect(add, &QPushButton::clicked, this, [=, this]() {
        if (name->currentIndex() < 0) return;
        QJsonObject item{{"Quantity", quantity->text().toInt()}, {"Price", price->text().toDouble()},
                         {"Name", name->currentText()}, {"Notes", notes->text()}};
        const auto selected = name->currentData().toJsonObject();
        if (!selected.isEmpty()) {
            item["StockID"] = selected.value("StockID");
            if (price->text().isEmpty()) item["Price"] = selected.value("Price");
        }
        quantity->clear();
        price->clear();
        notes->clear();
        stock_items.append(item);
        emit add_item_to_invoice(item);
    });
    connect(create_invoice_pdf, &QPushButton::clicked, this, [this]() {
        if (stock_items.empty()) return;
        emit add_item_list_to_invoice(QJsonDocument(stock_items));
        create_invoice_pdf->setDisabled(true);
    });
    return form;
}
