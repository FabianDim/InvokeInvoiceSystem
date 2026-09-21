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
    ++stock_request_;
    set_saving_stock(false);
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
    new_name_->clear();
    unit_->clear();
    keywords_->clear();
    margin_->setValue(0);
    stock_on_hand_->setValue(0);
    save_stock_->setChecked(false);
    item_source_->setCurrentIndex(0);
    status_label_->clear();
    update_item_source();
}

void NewInvoiceStock::business_selected(const QJsonObject& business) {
    const auto id = business.value("BusinessID").toString();
    if (id != business_id_ || id.isEmpty()) {
        business_id_ = id;
        reset_invoice();
    }
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
    back_button_ = new QPushButton("Back to dashboard", content);
    UiStyle::button(create_invoice_pdf, "primary");
    UiStyle::button(back_button_);
    layout->addLayout(create_item_entry_form());
    status_label_ = UiStyle::label("", content, "status");
    status_label_->setObjectName("invoice_stock_status");
    status_label_->setTextFormat(Qt::PlainText);
    status_label_->setWordWrap(true);
    layout->addWidget(status_label_);
    auto* buttons = new QHBoxLayout;
    buttons->setSpacing(12);
    buttons->addWidget(back_button_, 1);
    buttons->addWidget(create_invoice_pdf, 1);
    layout->addLayout(buttons);
    connect(back_button_, &QPushButton::clicked, this, [this]() { emit invoice_navigation(Page::Dashboard); });
}

QLayout* NewInvoiceStock::create_item_entry_form() {
    auto* container = new QVBoxLayout;
    entry_widget_ = new QWidget(this);
    container->addWidget(entry_widget_);
    auto* form = new QGridLayout(entry_widget_);
    form->setContentsMargins(0, 0, 0, 0);
    form->setHorizontalSpacing(12);
    form->setVerticalSpacing(8);
    form->setColumnStretch(0, 1);
    form->setColumnStretch(1, 1);
    auto* name = new QComboBox(this);
    name->setObjectName("invoice_saved_stock");
    name->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    item_source_ = new QComboBox(this);
    item_source_->setObjectName("invoice_item_source");
    item_source_->addItems({"New item", "Saved stock"});
    new_name_ = new QLineEdit(this);
    new_name_->setObjectName("invoice_new_stock_name");
    new_name_->setPlaceholderText("Item name or description");
    auto* quantity = new QLineEdit(this);
    auto* price = new QLineEdit(this);
    auto* notes = new QLineEdit(this);
    auto* add = new QPushButton("Add item", this);
    add->setObjectName("invoice_add_item");
    UiStyle::button(add);
    quantity->setValidator(new QIntValidator(1, 1'000'000, quantity));
    price->setValidator(new QDoubleValidator(0, 1e9, 2, price));
    quantity->setPlaceholderText("Quantity");
    price->setPlaceholderText("Price each");
    notes->setPlaceholderText("Optional notes");
    const auto add_field = [this, form](const QString& text, QWidget* edit, int row, int column) {
        auto* label = UiStyle::label(text, this);
        label->setBuddy(edit);
        edit->setProperty("role", "input");
        edit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        form->addWidget(label, row, column);
        form->addWidget(edit, row + 1, column);
    };
    add_field("Item source", item_source_, 0, 0);
    add_field("Item", new_name_, 0, 1);
    name->setProperty("role", "input");
    form->addWidget(name, 1, 1);
    add_field("Invoice quantity", quantity, 2, 0);
    add_field("Price each", price, 2, 1);
    add_field("Notes (optional)", notes, 4, 0);
    save_stock_ = new QCheckBox("Save this item to business stock", this);
    save_stock_->setObjectName("save_invoice_stock");
    form->addWidget(save_stock_, 5, 1);
    saved_fields_ = new QWidget(this);
    auto* saved_layout = new QGridLayout(saved_fields_);
    saved_layout->setContentsMargins(0, 0, 0, 0);
    stock_on_hand_ = new QSpinBox(saved_fields_);
    stock_on_hand_->setRange(0, 1'000'000);
    stock_on_hand_->setObjectName("new_stock_on_hand");
    margin_ = new QDoubleSpinBox(saved_fields_);
    margin_->setRange(0, 1'000'000);
    margin_->setObjectName("new_stock_margin");
    unit_ = new QLineEdit(saved_fields_);
    unit_->setObjectName("new_stock_unit");
    unit_->setPlaceholderText("Defaults to each");
    keywords_ = new QLineEdit(saved_fields_);
    keywords_->setObjectName("new_stock_keywords");
    keywords_->setPlaceholderText("Optional, comma separated");
    const QList<QPair<QString, QWidget*>> saved_inputs{
        {"Stock on hand", stock_on_hand_}, {"Margin", margin_}, {"Unit", unit_}, {"Keywords", keywords_}};
    for (int index = 0; index < saved_inputs.size(); ++index) {
        auto* label = UiStyle::label(saved_inputs[index].first + " (optional)", saved_fields_);
        label->setBuddy(saved_inputs[index].second);
        saved_inputs[index].second->setProperty("role", "input");
        saved_layout->addWidget(label, (index / 2) * 2, index % 2);
        saved_layout->addWidget(saved_inputs[index].second, (index / 2) * 2 + 1, index % 2);
    }
    form->addWidget(saved_fields_, 6, 0, 1, 2);
    form->addWidget(add, 7, 1);
    invoice_body_form_fields["stock_selector"] = name;
    invoice_body_form_fields["quantity"] = quantity;
    invoice_body_form_fields["price"] = price;
    invoice_body_form_fields["notes"] = notes;
    connect(item_source_, &QComboBox::currentIndexChanged, this, &NewInvoiceStock::update_item_source);
    connect(save_stock_, &QCheckBox::toggled, this, &NewInvoiceStock::update_item_source);
    update_item_source();

    connect(add, &QPushButton::clicked, this, [=, this]() {
        if (saving_stock_) return;
        const bool is_new = item_source_->currentIndex() == 0;
        const auto selected = name->currentData().toJsonObject();
        if (!quantity->hasAcceptableInput() || (is_new && new_name_->text().trimmed().isEmpty()) ||
            (!is_new && selected.isEmpty()) ||
            ((is_new || !price->text().isEmpty()) && !price->hasAcceptableInput())) {
            status_label_->setText("Enter an item name, a positive invoice quantity, and a valid price.");
            return;
        }
        const auto* price_validator = qobject_cast<const QDoubleValidator*>(price->validator());
        const auto item_price = price->text().isEmpty() ? selected.value("Price").toDouble()
                                                       : price_validator->locale().toDouble(price->text());
        const auto* quantity_validator = qobject_cast<const QIntValidator*>(quantity->validator());
        QJsonObject item{{"Quantity", quantity_validator->locale().toInt(quantity->text())}, {"Price", item_price},
                         {"Name", is_new ? new_name_->text().trimmed() : selected.value("Name").toString()},
                         {"Notes", notes->text()}};
        if (!is_new)
            item["StockID"] = selected.value("StockID");
        if (is_new && save_stock_->isChecked()) {
            item["StockOnHand"] = stock_on_hand_->value();
            item["Margin"] = margin_->value();
            item["Unit"] = unit_->text().trimmed().isEmpty() ? "each" : unit_->text().trimmed();
            item["Keywords"] = keywords_->text().trimmed();
            set_saving_stock(true);
            status_label_->setText(offline_ ? "Adding demo stock..." : "Saving stock item...");
            emit save_invoice_stock(QJsonDocument(item), ++stock_request_);
            return;
        }
        append_item(item);
        status_label_->setText("Item added to this invoice.");
    });
    connect(create_invoice_pdf, &QPushButton::clicked, this, [this]() {
        if (stock_items.empty() || saving_stock_) return;
        create_invoice_pdf->setDisabled(true);
        emit add_item_list_to_invoice(QJsonDocument(stock_items));
    });
    return container;
}

void NewInvoiceStock::update_item_source() {
    const bool is_new = item_source_->currentIndex() == 0;
    new_name_->setVisible(is_new);
    invoice_body_form_fields.value("stock_selector")->setVisible(!is_new);
    save_stock_->setVisible(is_new);
    saved_fields_->setVisible(is_new && save_stock_->isChecked());
    qobject_cast<QLineEdit*>(invoice_body_form_fields.value("price"))->setPlaceholderText(is_new ? "Price each" : "Use stock price");
}

void NewInvoiceStock::set_saving_stock(bool saving) {
    saving_stock_ = saving;
    entry_widget_->setEnabled(!saving);
    back_button_->setEnabled(!saving);
    create_invoice_pdf->setEnabled(!saving);
}

void NewInvoiceStock::append_item(const QJsonObject& item) {
    stock_items.append(item);
    emit add_item_to_invoice(item);
    for (const auto& key : {"quantity", "price", "notes"})
        qobject_cast<QLineEdit*>(invoice_body_form_fields.value(key))->clear();
    new_name_->clear();
    save_stock_->setChecked(false);
    stock_on_hand_->setValue(0);
    margin_->setValue(0);
    unit_->clear();
    keywords_->clear();
}

void NewInvoiceStock::stock_saved(const QJsonDocument& item, quint64 request_id) {
    if (!saving_stock_ || request_id != stock_request_) return;
    set_saving_stock(false);
    append_item(item.object());
    auto saved = item.object();
    saved["Quantity"] = saved.value("StockOnHand");
    auto* selector = qobject_cast<QComboBox*>(invoice_body_form_fields.value("stock_selector"));
    selector->addItem(saved.value("Name").toString(), saved);
    status_label_->setText(offline_ ? "Demo stock added to this invoice. It will be cleared when you exit the demo."
                                    : "Stock item saved and added to this invoice.");
}

void NewInvoiceStock::stock_save_failed(const QString& message, quint64 request_id) {
    if (!saving_stock_ || request_id != stock_request_) return;
    set_saving_stock(false);
    status_label_->setText(message + " Retry, or untick Save to add it only to this invoice.");
}

void NewInvoiceStock::set_offline(bool offline) {
    offline_ = offline;
    reset_invoice();
    item_source_->setItemText(1, offline ? "Demo stock" : "Saved stock");
    save_stock_->setText(offline ? "Keep this item in demo stock" : "Save this item to business stock");
}

void NewInvoiceStock::pdf_generated(const QString& path) {
    status_label_->setText("PDF saved to " + path);
}

void NewInvoiceStock::pdf_failed(const QString& message) {
    create_invoice_pdf->setEnabled(true);
    status_label_->setText(message);
}
