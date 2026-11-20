#include "Domain/Stock/StockEdit.h"
#include <QFormLayout>
#include <QLineEdit>
#include <QLocale>
#include <QMap>
#include "Infrastructure/Logging/Logging.h"
#include "View/UICode/Views/ItemsPage.h"
#include "View/UiStyle.h"
#include <QHeaderView>
#include <QJsonArray>
#include <QVBoxLayout>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>

using namespace App::Views;

ItemsPage::ItemsPage(QWidget* parent) : QWidget(parent) {
    create_page_layout();
    business_selected(QJsonObject{});
}

void ItemsPage::create_page_layout() {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(12);
    layout->addWidget(UiStyle::label("Business records", this, "title"));
    business_label_ = UiStyle::label("", this, "subtitle");
    business_label_->setTextFormat(Qt::PlainText);
    business_label_->setWordWrap(true);
    layout->addWidget(business_label_);

    tabs_ = new QTabWidget(this);
    tabs_->setObjectName("business_items_tabs");
    tabs_->setTabPosition(QTabWidget::North);
    const auto add_table = [this](const QString& title, const QString& name, const QStringList& headers) {
        auto* table = new QTableWidget(0, headers.size(), tabs_);
        table->setObjectName(name);
        table->setAccessibleName(title);
        table->setHorizontalHeaderLabels(headers);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setAlternatingRowColors(true);
        table->setSortingEnabled(true);
        table->verticalHeader()->hide();
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        table->horizontalHeader()->setStretchLastSection(true);
        table->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(table, &QTableWidget::customContextMenuRequested, this, [this, table](const QPoint& point) {
            auto* cell = table->itemAt(point);
            if (!cell) return;
            const auto id = cell->data(Qt::UserRole).toString();
            if (id.isEmpty()) return;
            QMenu menu(table);
            auto* edit = table == stock_table_ ? menu.addAction("Edit stock item") : nullptr;
            auto* view = menu.addAction("View record ID");
            auto* copy = menu.addAction("Copy record ID");
            auto* selected = menu.exec(table->viewport()->mapToGlobal(point));
            if (edit && selected == edit) edit_stock(cell->row());
            if (selected == copy) QApplication::clipboard()->setText(id);
            if (selected == view) {
                QMessageBox dialog(QMessageBox::Information, "Record ID", id, QMessageBox::Ok, this);
                dialog.setTextInteractionFlags(Qt::TextSelectableByMouse);
                dialog.exec();
            }
        });
        tabs_->addTab(table, title);
        return table;
    };
    business_table_ = add_table("Business", "business_records",
                                {"Name", "ABN", "ACN", "Phone", "Address", "Website", "Logo path"});
    clients_table_ = add_table("Clients", "client_records", {"Name", "Phone", "Email", "Address"});
    stock_table_ = add_table("Stock", "stock_records",
                             {"Name", "Quantity", "Price", "Margin", "Unit", "Keywords"});
    connect(stock_table_, &QTableWidget::cellDoubleClicked, this, [this](int row, int) { edit_stock(row); });
    stock_table_->setSelectionMode(QAbstractItemView::SingleSelection);
    layout->addWidget(tabs_, 1);

    status_label_ = UiStyle::label("", this, "status");
    status_label_->setObjectName("items_status");
    status_label_->setTextFormat(Qt::PlainText);
    status_label_->setWordWrap(true);
    layout->addWidget(status_label_);
    auto* buttons = new QHBoxLayout;
    auto* back = new QPushButton("Back to home", this);
    UiStyle::button(back);
    refresh_button_ = new QPushButton("Refresh", this);
    refresh_button_->setObjectName("refresh_items");
    UiStyle::button(refresh_button_, "primary");
    edit_stock_button_ = new QPushButton("Edit stock item", this);
    edit_stock_button_->setObjectName("edit_stock_item");
    UiStyle::button(edit_stock_button_);
    edit_stock_button_->setEnabled(false);
    connect(stock_table_, &QTableWidget::itemSelectionChanged, this, &ItemsPage::update_edit_button);
    connect(tabs_, &QTabWidget::currentChanged, this, &ItemsPage::update_edit_button);
    connect(edit_stock_button_, &QPushButton::clicked, this, [this]() { edit_stock(stock_table_->currentRow()); });
    buttons->addWidget(back);
    buttons->addWidget(edit_stock_button_);
    buttons->addWidget(refresh_button_);
    layout->addLayout(buttons);
    connect(back, &QPushButton::clicked, this, [this]() { emit navigate_to(Page::Dashboard); });
    connect(refresh_button_, &QPushButton::clicked, this, [this]() { load_items(true); });
}

void ItemsPage::business_selected(const QJsonObject& business) {
    ::Infrastructure::Logging::Operation operation(::Infrastructure::Logging::ui(), "ItemsPage.business_selected");
    const auto id = business.value("_id").toString();
    business_label_->setText(id.isEmpty() ? "Choose a business on the home page."
                                        : business.value("BusinessName").toString("Business"));
    if (id != business_id_ || id.isEmpty()) {
        ++stock_edit_request_;
        if (stock_editor_) stock_editor_->reject();
        business_id_ = id;
        for (auto* table : {business_table_, clients_table_, stock_table_})
            table->setRowCount(0);
        tabs_->setTabText(0, "Business");
        tabs_->setTabText(1, "Clients");
        tabs_->setTabText(2, "Stock");
        status_label_->clear();
        tabs_->setCurrentIndex(0);
    }
    refresh_button_->setEnabled(!id.isEmpty());
    update_edit_button();
}

void ItemsPage::load_items(bool refresh) {
    ::Infrastructure::Logging::Operation operation(::Infrastructure::Logging::ui(), "ItemsPage.load_items");
    if (business_id_.isEmpty())
        return;
    status_label_->setText("Loading business records...");
    refresh_button_->setEnabled(false);
    update_edit_button();
    emit items_requested(refresh);
}

void ItemsPage::populate_table(QTableWidget* table, const QJsonArray& items, const QStringList& fields) {
    table->setSortingEnabled(false);
    table->setRowCount(items.size());
    for (int row = 0; row < items.size(); ++row) {
        const auto record = items.at(row).toObject();
        for (int column = 0; column < fields.size(); ++column) {
            const auto value = record.value(fields.at(column));
            auto* cell = new QTableWidgetItem;
            if (value.isArray()) {
                QStringList values;
                for (const auto& entry : value.toArray())
                    values.append(entry.toString());
                cell->setText(values.join(", "));
            } else {
                cell->setData(Qt::DisplayRole, value.toVariant());
            }
            if (fields.at(column) == "Margin")
                cell->setText(QString::number(value.toDouble() * 100, 'g', 8) + "%");
            cell->setData(Qt::UserRole, record.value("_id").toString());
            cell->setData(Qt::UserRole + 1, record);
            cell->setToolTip(cell->text());
            table->setItem(row, column, cell);
        }
    }
    table->setSortingEnabled(true);
    table->resizeColumnsToContents();
    for (int column = 0; column < table->columnCount(); ++column)
        table->setColumnWidth(column, qBound(100, table->columnWidth(column), 300));
}

void ItemsPage::populate_items(const QJsonDocument& items) {
    ::Infrastructure::Logging::Operation operation(::Infrastructure::Logging::ui(), "ItemsPage.populate_items");
    const auto data = items.object();
    if (business_id_.isEmpty() || data.value("BusinessID").toString() != business_id_)
        return;
    const auto businesses = data.value("businesses").toArray();
    const auto clients = data.value("clients").toArray();
    const auto stock = data.value("stocks").toArray();
    populate_table(business_table_, businesses, {"BusinessName", "ABN", "ACN", "Phone", "BusinessAddress", "Website", "LogoPath"});
    populate_table(clients_table_, clients, {"Name", "Phone", "Email", "Address"});
    populate_table(stock_table_, stock, {"Name", "Quantity", "Price", "Margin", "Unit", "Keywords"});
    tabs_->setTabText(0, QString("Business (%1)").arg(businesses.size()));
    tabs_->setTabText(1, QString("Clients (%1)").arg(clients.size()));
    tabs_->setTabText(2, QString("Stock (%1)").arg(stock.size()));
    status_label_->setText("Records loaded. Empty tabs have no saved records. Use Refresh to check for changes.");
    refresh_button_->setEnabled(true);
    update_edit_button();
}

void ItemsPage::set_error(const QString& message) {
    ::Infrastructure::Logging::Operation operation(::Infrastructure::Logging::ui(), "ItemsPage.set_error");
    status_label_->setText("Could not load records: " + message + " Use Refresh to try again.");
    refresh_button_->setEnabled(!business_id_.isEmpty());
    update_edit_button();
}

void ItemsPage::update_edit_button() {
    edit_stock_button_->setEnabled(!business_id_.isEmpty() && refresh_button_->isEnabled() &&
        tabs_->currentWidget() == stock_table_ && !stock_table_->selectedItems().isEmpty());
}

void ItemsPage::edit_stock(int row) {
    if (business_id_.isEmpty() || !refresh_button_->isEnabled() || !stock_table_->item(row, 0)) return;
    if (stock_editor_) { stock_editor_->raise(); return; }
    const auto record = stock_table_->item(row, 0)->data(Qt::UserRole + 1).toJsonObject();
    if (record.value("_id").toString().isEmpty()) return;
    auto* dialog = new QDialog(this);
    stock_editor_ = dialog;
    ++stock_edit_request_;
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setObjectName("stock_edit_dialog");
    dialog->setWindowTitle("Edit stock item");
    dialog->setWindowModality(Qt::WindowModal);
    dialog->resize(540, 460);
    auto* layout = new QVBoxLayout(dialog);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(12);
    layout->addWidget(UiStyle::label("Edit stock item", dialog, "title"));
    auto* form = new QFormLayout;
    form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    QMap<QString, QLineEdit*> inputs;
    const QList<QPair<QString, QString>> fields{{"Name", "Name"}, {"Quantity", "Stock on hand"},
        {"Price", "Price (excluding GST)"}, {"Margin", "Margin (%)"}, {"Unit", "Unit"}, {"Keywords", "Keywords (comma-separated)"}};
    for (const auto& [key, label] : fields) {
        auto* input = new QLineEdit(dialog);
        input->setObjectName("stock_edit_" + key.toLower());
        input->setProperty("role", "input");
        if (key == "Quantity" || key == "Price" || key == "Margin")
            input->setText(QLocale().toString(record.value(key).toDouble() * (key == "Margin" ? 100 : 1), 'g', 15));
        else if (key == "Keywords") {
            QStringList words;
            for (const auto& word : record.value(key).toArray()) words.append(word.toString());
            input->setText(words.join(", "));
        } else input->setText(record.value(key).toString());
        input->setMaxLength(key == "Keywords" ? 2000 : key == "Unit" ? 80 : 200);
        auto* field_label = UiStyle::label(label, dialog); field_label->setBuddy(input);
        form->addRow(field_label, input);
        inputs[key] = input;
    }
    layout->addLayout(form);
    auto* status = UiStyle::label("", dialog, "status");
    status->setObjectName("stock_edit_status");
    status->setTextFormat(Qt::PlainText); status->setWordWrap(true);
    layout->addWidget(status);
    auto* buttons = new QHBoxLayout;
    auto* cancel = new QPushButton("Cancel", dialog);
    auto* save = new QPushButton("Save changes", dialog);
    cancel->setObjectName("stock_edit_cancel"); save->setObjectName("stock_edit_save");
    UiStyle::button(cancel); UiStyle::button(save, "primary");
    buttons->addWidget(cancel); buttons->addWidget(save); layout->addLayout(buttons);
    connect(cancel, &QPushButton::clicked, dialog, &QDialog::reject);
    const auto business = business_id_;
    connect(save, &QPushButton::clicked, this, [=, this]() {
        QJsonObject data{{"_id", record.value("_id")}, {"BusinessID", business}, {"ExpectedQuantity", record.value("Quantity")}};
        for (auto it = inputs.begin(); it != inputs.end(); ++it) data[it.key()] = it.value()->text().trimmed();
        for (const auto* key : {"Quantity", "Price", "Margin"}) {
            bool valid = false;
            const auto text = inputs[key]->text().trimmed();
            const auto number = QLocale().toDouble(text, &valid);
            if (!valid) { status->setText(QString("Enter a valid %1.").arg(QString(key).toLower())); inputs[key]->setFocus(); return; }
            data[key] = QString(key) == "Margin" ? number / 100 : number;
        }
        try { Domain::Stock::stock_edit_values(data); }
        catch (const std::exception& error) { status->setText(QString::fromUtf8(error.what())); return; }
        status->setText("Saving changes...");
        dialog->setEnabled(false);
        emit stock_update_requested(data, ++stock_edit_request_);
    });
    dialog->open();
    inputs["Name"]->setFocus();
}

void ItemsPage::stock_updated(quint64 request_id) {
    if (request_id != stock_edit_request_) return;
    if (stock_editor_) stock_editor_->accept();
    load_items(true);
}

void ItemsPage::stock_update_failed(const QString& message, quint64 request_id) {
    if (request_id != stock_edit_request_ || !stock_editor_) return;
    stock_editor_->setEnabled(true);
    stock_editor_->findChild<QLabel*>("stock_edit_status")->setText(message);
}
