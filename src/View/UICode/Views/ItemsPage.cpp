#include "View/UICode/Views/ManagementForm.h"
#include <QScrollArea>
#include <QGuiApplication>
#include <QScreen>
#include "Domain/Records/RecordDefinition.h"
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
        table->setSelectionMode(QAbstractItemView::SingleSelection);
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
            auto* edit = menu.addAction(table == stock_table_ ? "Edit stock item" : table == clients_table_ ? "Edit client" : "Edit business");
            edit->setEnabled(refresh_button_->isEnabled());
            auto* view = menu.addAction("View record ID");
            auto* copy = menu.addAction("Copy record ID");
            auto* selected = menu.exec(table->viewport()->mapToGlobal(point));
            if (selected == edit) edit_record(table, cell->row());
            if (selected == copy) QApplication::clipboard()->setText(id);
            if (selected == view) {
                QMessageBox dialog(QMessageBox::Information, "Record ID", id, QMessageBox::Ok, this);
                dialog.setTextInteractionFlags(Qt::TextSelectableByMouse);
                dialog.exec();
            }
        });
        connect(table, &QTableWidget::cellDoubleClicked, this, [this, table](int row, int) { edit_record(table, row); });
        connect(table, &QTableWidget::itemSelectionChanged, this, &ItemsPage::update_edit_button);
        tabs_->addTab(table, title);
        return table;
    };
    business_table_ = add_table("Business", "business_records",
                                {"Name", "ABN", "ACN", "Phone", "Address", "Website", "Logo path"});
    clients_table_ = add_table("Clients", "client_records", {"Name", "Phone", "Email", "Address"});
    stock_table_ = add_table("Stock", "stock_records",
                             {"Name", "Quantity", "Price", "Margin", "Unit", "Keywords"});
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
    edit_record_button_ = new QPushButton("Edit business", this);
    edit_record_button_->setObjectName("edit_record");
    UiStyle::button(edit_record_button_);
    edit_record_button_->setEnabled(false);
    connect(tabs_, &QTabWidget::currentChanged, this, &ItemsPage::update_edit_button);
    connect(edit_record_button_, &QPushButton::clicked, this, [this]() {
        auto* table = qobject_cast<QTableWidget*>(tabs_->currentWidget());
        if (table) edit_record(table, table->currentRow());
    });
    buttons->addWidget(back);
    buttons->addWidget(edit_record_button_);
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
        ++record_edit_request_;
        if (record_editor_) record_editor_->reject();
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
    if (!businesses.isEmpty()) business_label_->setText(businesses.first().toObject().value("BusinessName").toString());
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
    auto* table = qobject_cast<QTableWidget*>(tabs_->currentWidget());
    edit_record_button_->setText(table == stock_table_ ? "Edit stock item" : table == clients_table_ ? "Edit client" : "Edit business");
    edit_record_button_->setEnabled(!business_id_.isEmpty() && refresh_button_->isEnabled() &&
        table && !table->selectedItems().isEmpty());
}

void ItemsPage::edit_record(QTableWidget* table, int row) {
    if (business_id_.isEmpty() || !refresh_button_->isEnabled() || !table->item(row, 0)) return;
    if (record_editor_) { record_editor_->raise(); return; }
    const auto record = table->item(row, 0)->data(Qt::UserRole + 1).toJsonObject();
    const QString resource = table == stock_table_ ? "stock" : table == clients_table_ ? "client" : "business";
    const auto& definition = Domain::Records::RecordDefinition::for_resource(resource);
    if (record.value("_id").toString().isEmpty()) return;
    auto* dialog = new QDialog(this);
    record_editor_ = dialog;
    ++record_edit_request_;
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setObjectName(resource + "_edit_dialog");
    dialog->setWindowTitle("Edit " + definition.noun());
    dialog->setWindowModality(Qt::WindowModal);
    const auto available = dialog->screen()->availableGeometry();
    dialog->resize(qMin(640, available.width()), qMin(definition.is_business() ? 760 : 540, available.height() - 60));
    auto* layout = new QVBoxLayout(dialog);
    layout->setContentsMargins(0, 0, 0, 0);
    auto* scroll = new QScrollArea(dialog);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setWidgetResizable(true);
    auto* form = new ManagementForm(definition, record);
    scroll->setWidget(form);
    layout->addWidget(scroll);
    connect(form, &ManagementForm::navigate_to, dialog, &QDialog::reject);
    const auto business = business_id_;
    connect(form, &ManagementForm::submit_resource, this, [this, dialog, business](const QString& resource, const QJsonDocument& values) {
        auto data = values.object();
        data["BusinessID"] = business;
        dialog->setEnabled(false);
        emit record_update_requested(resource, data, ++record_edit_request_);
    });
    dialog->open();
}

void ItemsPage::record_updated(quint64 request_id) {
    if (request_id != record_edit_request_) return;
    if (record_editor_) record_editor_->accept();
    load_items(true);
}

void ItemsPage::record_update_failed(const QString& message, quint64 request_id) {
    if (request_id != record_edit_request_ || !record_editor_) return;
    record_editor_->setEnabled(true);
    record_editor_->findChild<ManagementForm*>()->save_failed(message);
}
