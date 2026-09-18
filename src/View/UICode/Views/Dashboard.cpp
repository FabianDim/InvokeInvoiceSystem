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
    auto* content = UiStyle::page_content(this, 640);
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
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title, 0, 0, 1, 2);
    auto* subtitle = new QLabel("Create invoices from the business data you already manage.", content);
    subtitle->setObjectName("subtitleLabel");
    subtitle->setProperty("role", "subtitle");
    subtitle->setWordWrap(true);
    subtitle->setAlignment(Qt::AlignCenter);
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

    QIcon icon(":/icons/invoice_icon.png");
    invoice_button = new QPushButton(icon, "Create new invoice", content);
    client_button = new QPushButton("Create new client", content);
    business_button = new QPushButton("Configure business", content);
    stock_button = new QPushButton("Create stock item", content);
    account_button = new QPushButton("Account settings", content);
    items_button = new QPushButton("Browse business records", content);
    items_button->setObjectName("browse_items_button");

    invoice_button->setObjectName("primary_dashboard_button");
    client_button->setObjectName("dashboard_button");
    business_button->setObjectName("dashboard_button");
    stock_button->setObjectName("dashboard_button");
    account_button->setObjectName("dashboard_button");
    UiStyle::button(invoice_button, "accent", true);
    for (auto* button : {client_button, business_button, stock_button, account_button, items_button})
        UiStyle::button(button, "primary", true);
    layout->addWidget(invoice_button, 3, 0);
    layout->addWidget(client_button, 3, 1);
    layout->addWidget(business_button, 4, 0);
    layout->addWidget(stock_button, 4, 1);
    layout->addWidget(items_button, 5, 0);
    layout->addWidget(account_button, 5, 1);

    button_group_->addButton(invoice_button);
    button_group_->addButton(client_button);
    button_group_->addButton(business_button);
    button_group_->addButton(stock_button);
    button_group_->addButton(account_button);
    button_group_->addButton(items_button);
}

void Dashboard::button_connections() {
    connect(business_select_, &QComboBox::currentIndexChanged, this, &Dashboard::update_business_selection);
    connect(invoice_button, &QPushButton::clicked, this, [this]() { emit dash_navigation(Page::NewInvoice); });
    connect(client_button, &QPushButton::clicked, this, [this]() { emit dash_navigation(Page::NewClient); });
    connect(business_button, &QPushButton::clicked, this, [this]() { emit dash_navigation(Page::BusinessSettings); });
    connect(stock_button, &QPushButton::clicked, this, [this]() { emit dash_navigation(Page::StockSettings); });
    connect(account_button, &QPushButton::clicked, this, [this]() { emit dash_navigation(Page::AccountSettings); });
    connect(items_button, &QPushButton::clicked, this, [this]() { emit dash_navigation(Page::Items); });
}

bool Dashboard::has_business() const {
    return !business_select_->currentData().toJsonObject().value("BusinessID").toString().isEmpty();
}

void Dashboard::populate_business_list(const QJsonDocument& list) {
    const auto previous_id = business_select_->currentData().toJsonObject().value("BusinessID").toString();
    {
        const QSignalBlocker blocker(business_select_);
        business_select_->clear();
        QVector<QJsonObject> businesses;
        const auto root = list.object();
        for (auto it = root.constBegin(); it != root.constEnd(); ++it) {
            auto business = it.value().toObject();
            if (business.value("BusinessID").toString().isEmpty())
                business["BusinessID"] = it.key();
            if (!business.value("BusinessID").toString().isEmpty())
                businesses.append(business);
        }
        std::sort(businesses.begin(), businesses.end(), [](const QJsonObject& a, const QJsonObject& b) {
            return a.value("BusinessName").toString().localeAwareCompare(b.value("BusinessName").toString()) < 0;
        });
        business_select_->addItem(businesses.isEmpty() ? "Create a business to get started" : "Choose a business");
        int selected_index = 0;
        for (const auto& business : businesses) {
            const auto id = business.value("BusinessID").toString();
            business_select_->addItem(business.value("BusinessName").toString(id) + " (" + id + ")", business);
            if (id == previous_id)
                selected_index = business_select_->count() - 1;
        }
        business_select_->setCurrentIndex(selected_index);
        business_select_->setEnabled(!businesses.isEmpty());
    }
    update_business_selection();
}

void Dashboard::update_business_selection() {
    const bool selected = has_business();
    invoice_button->setEnabled(selected);
    client_button->setEnabled(selected);
    stock_button->setEnabled(selected);
    items_button->setEnabled(selected);
    emit business_chosen(business_select_->currentData().toJsonObject());
}
