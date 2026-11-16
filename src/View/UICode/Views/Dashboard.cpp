#include "Infrastructure/Logging/Logging.h"
#include "View/UICode/Views/Dashboard.h"
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>
#include <QSignalBlocker>
#include "View/UiStyle.h"

using namespace App::Views;

App::Views::Dashboard::Dashboard(QWidget* parent) : QWidget(parent) {
    create_page_layout();
    button_connections();
    populate_business_list(QJsonDocument(QJsonObject{}));
}

void App::Views::Dashboard::create_page_layout() {
    auto* content = UiStyle::page_content(this, 920, false);
    button_group_ = new QButtonGroup(this);

    button_group_->setObjectName("dashboard_button_group");
    auto* layout = new QGridLayout(content);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);
    layout->setColumnStretch(0, 1);
    layout->setColumnStretch(1, 1);

    auto* title = new QLabel("Your workspace", content);
    title->setObjectName("titleLabel");
    title->setProperty("role", "title");
    title->setAlignment(Qt::AlignLeft);
    layout->addWidget(title, 0, 0, 1, 2);
    auto* subtitle = new QLabel("Create invoices from the business data you already manage.", content);
    subtitle->setObjectName("subtitleLabel");
    subtitle->setProperty("role", "subtitle");
    subtitle->setWordWrap(true);
    subtitle->setAlignment(Qt::AlignLeft);
    layout->addWidget(subtitle, 1, 0, 1, 2);

    auto* business_label = new QLabel("Active business", content);
    business_label->setObjectName("form_label");
    business_label->setProperty("role", "field");
    business_select_ = new QComboBox(content);
    business_select_->setObjectName("business_selector");
    business_select_->setProperty("role", "input");
    business_select_->setAccessibleName("Active business");
    business_select_->setMinimumContentsLength(18);
    business_select_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    business_label->setBuddy(business_select_);
    auto* business_row = new QHBoxLayout;
    business_row->addWidget(business_label);
    business_row->addSpacing(12);
    business_row->addWidget(business_select_, 1);
    layout->addLayout(business_row, 2, 0, 1, 2);

    invoice_button = new QPushButton("Create new invoice", content);
    client_button = new QPushButton("Create new client", content);
    business_button = new QPushButton("Configure business", content);
    stock_button = new QPushButton("Create stock item", content);
    account_button = new QPushButton("Account settings", content);
    default_invoice_settings_button = new QPushButton("Business invoice settings", content);
    default_invoice_settings_button->setObjectName("default_invoice_settings_button");
    default_invoice_settings_button->hide();
    items_button = new QPushButton("Browse business records", content);
    items_button->setObjectName("browse_items_button");
    invoices_button = new QPushButton("Browse invoices", content);
    invoices_button->setObjectName("browse_invoices_button");
    UiStyle::button(invoices_button, "secondary", true);

    invoice_button->setObjectName("primary_dashboard_button");
    client_button->setObjectName("dashboard_button");
    business_button->setObjectName("dashboard_button");
    stock_button->setObjectName("dashboard_button");
    account_button->setObjectName("dashboard_button");
    UiStyle::button(invoice_button, "accent", true);
    for (auto* button : {client_button, business_button, stock_button, account_button, items_button, default_invoice_settings_button})
        UiStyle::button(button, "secondary", true);
    const auto add_action = [layout, content](QPushButton* button, const QString& category,
                                            const QString& description, int row, int column) {
        auto* card = new QFrame(content);
        card->setObjectName(category == "06 / PERSONAL" ? "account_action_card" : "action_card");
        auto* card_layout = new QVBoxLayout(card);
        card_layout->setContentsMargins(18, 12, 18, 12);
        card_layout->setSpacing(6);
        card_layout->addWidget(UiStyle::label(category, card, "eyebrow"));
        card_layout->addWidget(button);
        auto* detail = UiStyle::label(description, card, "cardDetail");
        detail->setWordWrap(true);
        auto detail_policy = detail->sizePolicy();
        detail_policy.setHorizontalPolicy(QSizePolicy::Ignored);
        detail->setSizePolicy(detail_policy);
        card_layout->addWidget(detail);
        layout->addWidget(card, row, column);
    };
    add_action(invoice_button, "01 / INVOICING", "From work done to invoice sent.", 3, 0);
    add_action(client_button, "02 / CLIENTS", "A place for every working relationship.", 3, 1);
    add_action(business_button, "03 / BUSINESS", "Your business, down to the details.", 4, 0);
    add_action(stock_button, "04 / INVENTORY", "Products, quantities and pricing.", 4, 1);
    add_action(items_button, "05 / RECORDS", "Find the details you need, together.", 5, 0);
    add_action(account_button, "06 / PERSONAL", "A little housekeeping for your account.", 5, 1);
    layout->addWidget(invoices_button, 6, 0);
    layout->addWidget(default_invoice_settings_button, 6, 1);
    loading_label_ = UiStyle::label("", content, "status");
    loading_label_->setObjectName("business_loading_status");
    layout->addWidget(loading_label_, 7, 0, 1, 2);

    button_group_->addButton(invoice_button);
    button_group_->addButton(client_button);
    button_group_->addButton(business_button);
    button_group_->addButton(stock_button);
    button_group_->addButton(account_button);
    button_group_->addButton(items_button);
    button_group_->addButton(default_invoice_settings_button);
}

void Dashboard::button_connections() {
    connect(invoices_button, &QPushButton::clicked, this, [this]() { emit dash_navigation(Page::Invoices); });
    connect(default_invoice_settings_button, &QPushButton::clicked, this,
            [this]() { emit dash_navigation(Page::DefaultInvoiceSettings); });
    connect(business_select_, &QComboBox::currentIndexChanged, this, &Dashboard::update_business_selection);
    connect(invoice_button, &QPushButton::clicked, this, [this]() { emit dash_navigation(Page::NewInvoice); });
    connect(client_button, &QPushButton::clicked, this, [this]() { emit dash_navigation(Page::NewClient); });
    connect(business_button, &QPushButton::clicked, this, [this]() { emit dash_navigation(Page::BusinessSettings); });
    connect(stock_button, &QPushButton::clicked, this, [this]() { emit dash_navigation(Page::StockSettings); });
    connect(account_button, &QPushButton::clicked, this, [this]() { emit dash_navigation(Page::AccountSettings); });
    connect(items_button, &QPushButton::clicked, this, [this]() { emit dash_navigation(Page::Items); });
}

void Dashboard::set_offline(bool offline) {
    offline_ = offline;
    default_invoice_settings_button->setVisible(has_business());
    account_button->setVisible(!offline);
    findChild<QFrame*>("account_action_card")->setVisible(!offline);
    business_button->setText(offline ? "Create demo business" : "Configure business");
    items_button->setText(offline ? "Browse demo records" : "Browse business records");
    invoices_button->setText(offline ? "Browse demo invoices" : "Browse invoices");
    findChild<QLabel*>("titleLabel")->setText(offline ? "Try an offline invoice" : "Your workspace");
    findChild<QLabel*>("subtitleLabel")->setText(offline
        ? "Create a business, add a client and items, then export your PDF. Nothing is saved to your account."
        : "Create invoices from the business data you already manage.");
}

void Dashboard::set_logged_in(bool logged_in) {
    logged_in_ = logged_in;
    default_invoice_settings_button->setVisible(has_business());
}

bool Dashboard::has_business() const {
    return !business_select_->currentData().toJsonObject().value("_id").toString().isEmpty();
}

void Dashboard::populate_business_list(const QJsonDocument& list) {
    ::Infrastructure::Logging::Operation operation(::Infrastructure::Logging::ui(), "Dashboard.populate_business_list");
    const auto previous_id = pending_business_id_.isEmpty() ? business_select_->currentData().toJsonObject().value("_id").toString() : pending_business_id_;
    pending_business_id_.clear();
    {
        const QSignalBlocker blocker(business_select_);
        business_select_->clear();
        QVector<QJsonObject> businesses;
        const auto root = list.object();
        for (auto it = root.constBegin(); it != root.constEnd(); ++it) {
            auto business = it.value().toObject();
            if (business.value("_id").toString().isEmpty())
                business["_id"] = it.key();
            if (!business.value("_id").toString().isEmpty())
                businesses.append(business);
        }
        std::sort(businesses.begin(), businesses.end(), [](const QJsonObject& a, const QJsonObject& b) {
            return a.value("BusinessName").toString().localeAwareCompare(b.value("BusinessName").toString()) < 0;
        });
        business_select_->addItem(businesses.isEmpty() ? "Create a business to get started" : "Choose a business");
        int selected_index = 0;
        for (const auto& business : businesses) {
            const auto id = business.value("_id").toString();
            business_select_->addItem(business.value("BusinessName").toString("Business"), business);
            if (id == previous_id)
                selected_index = business_select_->count() - 1;
        }
        if (businesses.size() == 1)
            selected_index = 1;
        business_select_->setCurrentIndex(selected_index);
        business_select_->setEnabled(!businesses.isEmpty());
    }
    set_loading(false);
    update_business_selection();
}

void Dashboard::update_business_selection() {
    ::Infrastructure::Logging::Operation operation(::Infrastructure::Logging::ui(), "Dashboard.update_business_selection");
    const bool selected = has_business();
    default_invoice_settings_button->setVisible(selected);
    invoice_button->setEnabled(selected);
    client_button->setEnabled(selected);
    stock_button->setEnabled(selected);
    items_button->setEnabled(selected);
    invoices_button->setEnabled(selected);
    emit business_chosen(business_select_->currentData().toJsonObject());
}

void Dashboard::set_loading(bool loading) {
    loading_label_->setText(loading ? "Loading businesses..." : "");
    business_select_->setEnabled(!loading && business_select_->count() > 1);
}
void Dashboard::set_error(const QString& message) {
    set_loading(false);
    loading_label_->setText("Could not load businesses: " + message);
}
