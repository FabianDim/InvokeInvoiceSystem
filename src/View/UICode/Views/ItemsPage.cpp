#include "View/UICode/Views/ItemsPage.h"
#include "View/UiStyle.h"
#include <QHeaderView>
#include <QJsonArray>
#include <QVBoxLayout>

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
        tabs_->addTab(table, title);
        return table;
    };
    business_table_ = add_table("Business", "business_records",
                                {"Name", "Business ID", "ABN", "ACN", "Phone", "Address", "Website", "Logo path"});
    clients_table_ = add_table("Clients", "client_records", {"Name", "Client ID", "Phone", "Email", "Address"});
    stock_table_ = add_table("Stock", "stock_records",
                             {"Name", "Stock ID", "Quantity", "Price", "Margin", "Unit", "Keywords"});
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
    buttons->addWidget(back);
    buttons->addWidget(refresh_button_);
    layout->addLayout(buttons);
    connect(back, &QPushButton::clicked, this, [this]() { emit navigate_to(Page::Dashboard); });
    connect(refresh_button_, &QPushButton::clicked, this, [this]() { load_items(true); });
}

void ItemsPage::business_selected(const QJsonObject& business) {
    const auto id = business.value("BusinessID").toString();
    business_label_->setText(id.isEmpty() ? "Choose a business on the home page."
                                        : business.value("BusinessName").toString(id) + " (" + id + ")");
    if (id != business_id_ || id.isEmpty()) {
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
}

void ItemsPage::load_items(bool refresh) {
    if (business_id_.isEmpty())
        return;
    status_label_->setText("Loading business records...");
    refresh_button_->setEnabled(false);
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
    const auto data = items.object();
    if (business_id_.isEmpty() || data.value("BusinessID").toString() != business_id_)
        return;
    const auto businesses = data.value("businesses").toArray();
    const auto clients = data.value("clients").toArray();
    const auto stock = data.value("stocks").toArray();
    populate_table(business_table_, businesses, {"BusinessName", "BusinessID", "ABN", "ACN", "Phone", "BusinessAddress", "Website", "LogoPath"});
    populate_table(clients_table_, clients, {"Name", "ClientID", "Phone", "Email", "Address"});
    populate_table(stock_table_, stock, {"Name", "StockID", "Quantity", "Price", "Margin", "Unit", "Keywords"});
    tabs_->setTabText(0, QString("Business (%1)").arg(businesses.size()));
    tabs_->setTabText(1, QString("Clients (%1)").arg(clients.size()));
    tabs_->setTabText(2, QString("Stock (%1)").arg(stock.size()));
    status_label_->setText("Records loaded. Empty tabs have no saved records. Use Refresh to check for changes.");
    refresh_button_->setEnabled(true);
}

void ItemsPage::set_error(const QString& message) {
    status_label_->setText("Could not load records: " + message + " Use Refresh to try again.");
    refresh_button_->setEnabled(!business_id_.isEmpty());
}
