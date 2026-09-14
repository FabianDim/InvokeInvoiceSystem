#include "View/UICode/Views/Dashboard.h"
#include <QPushButton>
#include <QLabel>
#include <QGridLayout>

using namespace App::Views;

App::Views::Dashboard::Dashboard(QWidget* parent) : QWidget(parent) {
    create_page_layout();
    button_connections();
}

void App::Views::Dashboard::create_page_layout() {
    main_layout_ = new QHBoxLayout(this);
    main_layout_->setAlignment(Qt::AlignCenter);
    button_group_ = new QButtonGroup(main_layout_);

    button_group_->setObjectName("dashboard_button_group");
    auto* content = new QWidget(this);
    auto* layout = new QGridLayout(content);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(12);

    auto* title = new QLabel("Invoke Invoice System", content);
    title->setObjectName("titleLabel");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title, 0, 0, 1, 2);

    QIcon icon(":/icons/invoice_icon.png");
    invoice_button = new QPushButton(icon, "Create new invoice", content);
    client_button = new QPushButton("Create new client", content);
    business_button = new QPushButton("Configure business", content);
    stock_button = new QPushButton("Create stock item", content);
    account_button = new QPushButton("Account settings", content);

    layout->addWidget(invoice_button, 1, 0);
    layout->addWidget(client_button, 1, 1);
    layout->addWidget(business_button, 2, 0);
    layout->addWidget(stock_button, 2, 1);
    layout->addWidget(account_button, 3, 0, 1, 2);
    main_layout_->addWidget(content);

    button_group_->addButton(invoice_button);
    button_group_->addButton(client_button);
    button_group_->addButton(business_button);
    button_group_->addButton(stock_button);
    button_group_->addButton(account_button);
}

void Dashboard::button_connections() {
    connect(invoice_button, &QPushButton::clicked, this, [this]() { emit dash_navigation(Page::NewInvoice); });
    connect(client_button, &QPushButton::clicked, this, [this]() { emit dash_navigation(Page::NewClient); });
    connect(business_button, &QPushButton::clicked, this, [this]() { emit dash_navigation(Page::BusinessSettings); });
    connect(stock_button, &QPushButton::clicked, this, [this]() { emit dash_navigation(Page::StockSettings); });
    connect(account_button, &QPushButton::clicked, this, [this]() { emit dash_navigation(Page::AccountSettings); });
}