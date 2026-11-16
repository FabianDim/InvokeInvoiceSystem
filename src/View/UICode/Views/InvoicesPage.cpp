#include "View/UICode/Views/InvoicesPage.h"
#include "View/UiStyle.h"
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QDate>
#include <QTimer>
#include <QRegularExpression>
#include <QSignalBlocker>

using namespace App::Views;

InvoicesPage::InvoicesPage(QWidget* parent) : QWidget(parent) {
    auto* content = UiStyle::page_content(this);
    auto* layout = new QVBoxLayout(content);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(14);
    layout->addWidget(UiStyle::label("INVOICES AND PAYMENTS", content, "eyebrow"));
    layout->addWidget(UiStyle::label("Your invoices", content, "title"));
    detail_ = UiStyle::label("", content);
    detail_->setTextFormat(Qt::PlainText);
    detail_->setWordWrap(true);
    layout->addWidget(detail_);
    table_ = new QTableWidget(0, 6, content);
    table_->setObjectName("invoice_records");
    table_->setHorizontalHeaderLabels({"Invoice", "Client", "Issued", "Due date", "Total", "Payment"});
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_->setStyleSheet(
        "QTableWidget#invoice_records { selection-background-color: #274c3d; selection-color: #ffffff; }"
        "QTableWidget#invoice_records::item:selected { background-color: #274c3d; color: #ffffff; }"
        "QTableWidget#invoice_records::item:selected:!active { background-color: #274c3d; color: #ffffff; }");
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->verticalHeader()->hide();
    table_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    table_->setMinimumHeight(260);
    layout->addWidget(table_);
    auto* pager = new QHBoxLayout;
    previous_ = new QPushButton("Previous", content);
    next_ = new QPushButton("Next", content);
    refresh_ = new QPushButton("Refresh invoices", content);
    previous_->setObjectName("invoice_previous"); next_->setObjectName("invoice_next_page");
    refresh_->setObjectName("refresh_invoices");
    for (auto* button : {previous_, next_, refresh_}) { UiStyle::button(button); pager->addWidget(button); }
    layout->addLayout(pager);
    selection_ = UiStyle::label("", content);
    selection_->setObjectName("invoice_selection");
    selection_->setTextFormat(Qt::PlainText);
    selection_->setWordWrap(true);
    selection_->setStyleSheet("background-color: #e9efdf; color: #20362e; border: 1px solid #9caf91; border-radius: 7px; padding: 12px; font-weight: 600;");
    layout->addWidget(selection_);
    auto* actions = new QHBoxLayout;
    payment_ = new QPushButton("Mark as paid", content);
    export_ = new QPushButton("Save PDF", content);
    payment_->setObjectName("invoice_payment"); export_->setObjectName("invoice_export");
    UiStyle::button(payment_); UiStyle::button(export_, "primary");
    actions->addWidget(payment_); actions->addWidget(export_);
    layout->addLayout(actions);
    status_ = UiStyle::label("", content, "status");
    status_->setObjectName("invoice_records_status");
    status_->setTextFormat(Qt::PlainText); status_->setWordWrap(true);
    layout->addWidget(status_);
    auto* back = new QPushButton("Back to dashboard", content);
    UiStyle::button(back); layout->addWidget(back);
    connect(back, &QPushButton::clicked, this, [this]() { emit navigate_to(Page::Dashboard); });
    connect(refresh_, &QPushButton::clicked, this, [this]() { request_page(0); });
    connect(previous_, &QPushButton::clicked, this, [this]() { request_page(qMax(0, offset_ - 50)); });
    connect(next_, &QPushButton::clicked, this, [this]() { request_page(offset_ + 50); });
    connect(table_, &QTableWidget::itemSelectionChanged, this, &InvoicesPage::update_actions);
    connect(payment_, &QPushButton::clicked, this, [this]() {
        const auto record = selected();
        if (record.isEmpty() || busy_) return;
        busy_ = true; status_->setText("Saving payment status..."); update_actions();
        emit payment_requested(record.value("_id").toString(), !record.value("Paid").toBool());
    });
    connect(export_, &QPushButton::clicked, this, [this]() {
        const auto record = selected();
        if (record.isEmpty() || busy_ || !wait_.hasExpired()) return;
        auto name = record.value("InvoiceNumber").toString();
        name.replace(QRegularExpression("[^A-Za-z0-9._-]"), "_");
        const auto path = QFileDialog::getSaveFileName(this, "Save invoice PDF", name + ".pdf", "PDF files (*.pdf)");
        if (path.isEmpty()) return;
        busy_ = true; status_->setText("Regenerating PDF..."); update_actions();
        emit export_requested(record.value("_id").toString(), path.endsWith(".pdf", Qt::CaseInsensitive) ? path : path + ".pdf");
    });
    auto* timer = new QTimer(this);
    timer->setInterval(1000);
    connect(timer, &QTimer::timeout, this, [this]() {
        // Recompute overdue at midnight without needing a network refresh.
        for (int i = 0; i < records_.size(); ++i) {
            const auto record = records_[i].toObject();
            const auto due = QDate::fromString(record.value("DateDue").toString(), Qt::ISODate);
            const bool overdue = !record.value("Paid").toBool() && due.isValid() && due < QDate::currentDate();
            if (auto* cell = table_->item(i, 5)) {
                cell->setText(record.value("Paid").toBool() ? "Paid" : overdue ? "Unpaid - OVERDUE" : "Unpaid");
                cell->setForeground(overdue ? QColor("#a33524") : palette().text().color());
            }
        }
        update_actions();
    });
    timer->start();
    update_actions();
}

QJsonObject InvoicesPage::selected() const {
    const auto rows = table_->selectionModel()->selectedRows();
    const auto row = rows.isEmpty() ? -1 : rows.first().row();
    return row >= 0 && row < records_.size() ? records_.at(row).toObject() : QJsonObject{};
}

void InvoicesPage::business_selected(const QJsonObject& business) {
    const auto id = business.value("_id").toString();
    if (id == business_id_ && !id.isEmpty()) return;
    business_id_ = id; records_ = {}; offset_ = 0; more_ = false; busy_ = false;
    table_->setRowCount(0); status_->clear(); detail_->clear(); update_actions();
}

void InvoicesPage::load_invoices() {
    request_page(offset_);
}

void InvoicesPage::request_page(int offset) {
    if (business_id_.isEmpty() || busy_) return;
    busy_ = true; status_->setText("Loading invoices..."); update_actions();
    emit invoices_requested(offset);
}

void InvoicesPage::populate_invoices(const QJsonDocument& document) {
    const auto data = document.object();
    if (data.value("BusinessID").toString() != business_id_) return;
    busy_ = false; records_ = data.value("invoices").toArray();
    offset_ = data.value("offset").toInt(); more_ = data.value("has_more").toBool();
    status_->setText(records_.isEmpty() ? "No completed invoices on this page." : QString("Showing invoices %1-%2").arg(offset_ + 1).arg(offset_ + records_.size()));
    render();
}

void InvoicesPage::render() {
    const auto rows = table_->selectionModel()->selectedRows();
    const auto* previous = rows.isEmpty() ? nullptr : table_->item(rows.first().row(), 0);
    const auto old_id = previous ? previous->data(Qt::UserRole).toString() : QString{};
    const QSignalBlocker blocker(table_);
    table_->setRowCount(0); table_->setRowCount(records_.size());
    int selected_row = -1;
    for (int row = 0; row < records_.size(); ++row) {
        const auto record = records_[row].toObject();
        const auto date = [&](const char* key) { const auto parsed = QDate::fromString(record.value(key).toString(), Qt::ISODate);
            return parsed.isValid() ? parsed.toString("dd/MM/yyyy") : QString("Not recorded"); };
        const auto due = QDate::fromString(record.value("DateDue").toString(), Qt::ISODate);
        const bool overdue = !record.value("Paid").toBool() && due.isValid() && due < QDate::currentDate();
        const auto client_name = record.value("ClientName").toString();
        const QStringList cells{record.value("InvoiceNumber").toString(), client_name.isEmpty() ? "Not recorded" : client_name,
            date("DateCreated"), date("DateDue"), record.value("Total").isDouble() ? QString::number(record.value("Total").toDouble(), 'f', 2) : "Not recorded",
            record.value("Paid").toBool() ? "Paid" : overdue ? "Unpaid - OVERDUE" : "Unpaid"};
        for (int col = 0; col < cells.size(); ++col) {
            auto* item = new QTableWidgetItem(cells[col]);
            if (col == 0) item->setData(Qt::UserRole, record.value("_id").toString());
            if (col == 5 && overdue) item->setForeground(QColor("#a33524"));
            table_->setItem(row, col, item);
        }
        if (record.value("_id").toString() == old_id) selected_row = row;
    }
    if (selected_row >= 0) table_->selectRow(selected_row);
    update_actions();
}

void InvoicesPage::update_actions() {
    const auto record = selected();
    if (record.isEmpty()) {
        selection_->setText("Select an invoice above to update its payment status or save its PDF.");
    } else {
        const auto due = QDate::fromString(record.value("DateDue").toString(), Qt::ISODate);
        const bool overdue = !record.value("Paid").toBool() && due.isValid() && due < QDate::currentDate();
        const auto payment = record.value("Paid").toBool() ? "Paid" : overdue ? "Unpaid - OVERDUE" : "Unpaid";
        selection_->setText(QString("Selected invoice: %1\n%2 | %3")
            .arg(record.value("InvoiceNumber").toString(), record.value("ClientName").toString("Client not recorded"), payment));
    }
    refresh_->setEnabled(!busy_ && !business_id_.isEmpty());
    previous_->setEnabled(!busy_ && offset_ > 0); next_->setEnabled(!busy_ && more_);
    payment_->setEnabled(!busy_ && !record.isEmpty());
    payment_->setText(record.value("Paid").toBool() ? "Mark as unpaid" : "Mark as paid");
    export_->setEnabled(!busy_ && !record.isEmpty() && wait_.hasExpired());
    export_->setText(wait_.hasExpired() ? "Save PDF" : QString("Save PDF (%1s)").arg((wait_.remainingTime() + 999) / 1000));
    table_->setEnabled(!busy_);
    detail_->setText(record.value("LegacyCopy").toBool()
        ? "Older invoice: original dates and contact details were not archived. Its PDF uses current contact details; overdue status is unavailable without a due date."
        : "Saved invoices retain their original details. PDF regeneration does not change stock. A 10-second wait applies between exports.");
}

void InvoicesPage::set_error(const QString& message) { busy_ = false; status_->setText(message); update_actions(); }
void InvoicesPage::payment_updated(const QString& id, bool paid) {
    busy_ = false;
    QString number;
    for (int i = 0; i < records_.size(); ++i) {
        auto record = records_[i].toObject();
        if (record.value("_id").toString() == id) {
            number = record.value("InvoiceNumber").toString(); record["Paid"] = paid; records_[i] = record;
        }
    }
    status_->setText(QString("Invoice %1 marked as %2.").arg(number, paid ? "paid" : "unpaid")); render();
}
void InvoicesPage::export_finished(const QString& path) { busy_ = false; status_->setText("PDF saved to " + path); update_actions(); }
void InvoicesPage::export_wait(int seconds) { wait_ = QDeadlineTimer(seconds * 1000); update_actions(); }
