#include "View/UICode/Views/NewInvoiceCreation.h"
#include "View/UiStyle.h"
#include <QComboBox>
#include <QDateEdit>
#include <QFileDialog>
#include <QGridLayout>
#include <QJsonObject>
#include <QLineEdit>
#include <QJsonArray>
#include <QSignalBlocker>

using namespace App::Views;

InvoiceDetailsInput::InvoiceDetailsInput(QWidget* parent) : QWidget(parent) {
    create_page_layout();
    business_selected(QJsonObject{});
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
    auto* client_label = UiStyle::label("Client", content);
    client_select_ = new QComboBox(content);
    client_select_->setObjectName("invoice_client");
    client_select_->setProperty("role", "input");
    client_select_->setAccessibleName("Invoice client");
    client_select_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    client_select_->setMinimumContentsLength(16);
    client_select_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    client_label->setBuddy(client_select_);
    refresh_clients_ = new QPushButton("Refresh clients", content);
    refresh_clients_->setObjectName("refresh_invoice_clients");
    UiStyle::button(refresh_clients_);
    auto* client_row = new QHBoxLayout;
    client_row->setSpacing(8);
    client_row->addWidget(client_select_, 1);
    client_row->addWidget(refresh_clients_);
    form->addWidget(client_label, 0, 0);
    form->addLayout(client_row, 0, 1);
    client_details_ = UiStyle::label("", content);
    client_details_->setObjectName("invoice_client_details");
    client_details_->setTextFormat(Qt::PlainText);
    client_details_->setWordWrap(true);
    form->addWidget(client_details_, 1, 1);
    client_status_ = UiStyle::label("", content, "status");
    client_status_->setObjectName("invoice_client_status");
    client_status_->setTextFormat(Qt::PlainText);
    client_status_->setWordWrap(true);
    form->addWidget(client_status_, 2, 1);
    const std::vector<FormField> fields = {
        {"invoice_theme", "Invoice theme", FormField::Type::ComboBox, {}},
        {"invoice_number", "Invoice number", FormField::Type::LineEdit, {.placeholder = "e.g. INV-000123"}},
        {"invoice_file_name", "File name", FormField::Type::LineEdit, {.placeholder = "Name"}},
        {"invoice_file_dir", "Save PDF to", FormField::Type::DirBrowse, {}},
        {"date_created", "Date created", FormField::Type::DateEdit, {.defaultToday = true}},
        {"date_due", "Date due", FormField::Type::DateEdit, {.defaultToday = true}}};

    int row = 3;
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
    next_button_ = new QPushButton("Next", content);
    next_button_->setObjectName("invoice_next");
    UiStyle::button(back);
    UiStyle::button(next_button_, "primary");
    buttons->addWidget(back, 1);
    buttons->addWidget(next_button_, 1);
    layout->addLayout(buttons);
    connect(back, &QPushButton::clicked, this, [this]() { emit invoice_navigation(Page::Dashboard); });
    connect(client_select_, &QComboBox::currentIndexChanged, this, &InvoiceDetailsInput::update_client_selection);
    connect(refresh_clients_, &QPushButton::clicked, this, [this]() { load_clients(true); });
    connect(next_button_, &QPushButton::clicked, this, [this]() {
        const auto client_id = client_select_->currentData().toJsonObject().value("ClientID").toString();
        if (client_id.isEmpty())
            return;
        const auto text = [this](const QString& key) {
            return qobject_cast<QLineEdit*>(base_invoice_form_fields_.value(key))->text();
        };
        const auto date = [this](const QString& key) {
            return qobject_cast<QDateEdit*>(base_invoice_form_fields_.value(key))->date().toString(Qt::ISODate);
        };
        const auto theme = qobject_cast<QComboBox*>(base_invoice_form_fields_.value("invoice_theme"))->currentText();
        emit set_invoice_details(QJsonDocument(QJsonObject{
            {"ClientID", client_id},
            {"invoice_number", text("invoice_number")}, {"invoice_theme", theme},
            {"file_name", text("invoice_file_name")}, {"file_dir", text("invoice_file_dir")},
            {"date_created", date("date_created")}, {"date_due", date("date_due")}}));
    });
}

void InvoiceDetailsInput::business_selected(const QJsonObject& business) {
    const auto id = business.value("BusinessID").toString();
    if (id != business_id_ || id.isEmpty()) {
        business_id_ = id;
        client_select_->clear();
        client_select_->addItem("Choose a client");
        client_select_->setEnabled(false);
        client_status_->setText(id.isEmpty() ? "Choose a business on the dashboard first." : "");
        update_client_selection();
    }
    refresh_clients_->setEnabled(!id.isEmpty());
}

void InvoiceDetailsInput::load_clients(bool refresh) {
    if (business_id_.isEmpty())
        return;
    client_select_->clear();
    client_select_->addItem("Loading clients...");
    client_select_->setEnabled(false);
    refresh_clients_->setEnabled(false);
    client_status_->setText("Loading clients...");
    update_client_selection();
    emit clients_requested(refresh);
}

void InvoiceDetailsInput::populate_clients(const QJsonDocument& items) {
    const auto data = items.object();
    if (business_id_.isEmpty() || data.value("BusinessID").toString() != business_id_)
        return;
    const auto previous_id = client_select_->currentData().toJsonObject().value("ClientID").toString();
    {
        const QSignalBlocker blocker(client_select_);
        client_select_->clear();
        client_select_->addItem("Choose a client");
        QVector<QJsonObject> clients;
        for (const auto& value : data.value("clients").toArray()) {
            const auto client = value.toObject();
            if (!client.value("ClientID").toString().isEmpty())
                clients.append(client);
        }
        std::sort(clients.begin(), clients.end(), [](const QJsonObject& a, const QJsonObject& b) {
            return a.value("Name").toString().localeAwareCompare(b.value("Name").toString()) < 0;
        });
        for (const auto& client : clients) {
            const auto id = client.value("ClientID").toString();
            client_select_->addItem(client.value("Name").toString(id) + " (" + id + ")", client);
            if (id == previous_id)
                client_select_->setCurrentIndex(client_select_->count() - 1);
        }
    }
    const bool has_clients = client_select_->count() > 1;
    client_select_->setEnabled(has_clients);
    refresh_clients_->setEnabled(true);
    client_status_->setText(has_clients ? "" : "No clients saved for this business. Create a client from the dashboard first.");
    update_client_selection();
}

void InvoiceDetailsInput::update_client_selection() {
    const auto client = client_select_->currentData().toJsonObject();
    QStringList details;
    for (const auto& key : {"Address", "Email", "Phone"}) {
        if (!client.value(key).toString().isEmpty())
            details.append(client.value(key).toString());
    }
    client_details_->setText(details.join("\n"));
    next_button_->setEnabled(!client.value("ClientID").toString().isEmpty());
}

void InvoiceDetailsInput::set_client_error(const QString& message) {
    client_select_->clear();
    client_select_->addItem("Clients unavailable");
    client_select_->setEnabled(false);
    client_status_->setText("Could not load clients: " + message + " Use Refresh clients to try again.");
    refresh_clients_->setEnabled(!business_id_.isEmpty());
    update_client_selection();
}

void InvoiceDetailsInput::reset_form() {
    for (auto* field : base_invoice_form_fields_) {
        if (auto* edit = qobject_cast<QLineEdit*>(field))
            edit->clear();
        else if (auto* date = qobject_cast<QDateEdit*>(field))
            date->setDate(QDate::currentDate());
    }
    business_selected(QJsonObject{});
}
