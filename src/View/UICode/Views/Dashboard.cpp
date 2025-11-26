#include "View/UICode/Views/Dashboard.h"
#include <QPushButton>

using namespace App::Views;

App::Views::Dashboard::Dashboard(QWidget* parent) {
    create_page_layout();
    button_connections();
}

void App::Views::Dashboard::create_page_layout() {
    main_layout_ = new QHBoxLayout(this);
    main_layout_->setAlignment(Qt::AlignCenter);
    button_group_ = new QButtonGroup(main_layout_);

    button_group_->setObjectName("dashboard_button_group");
    QIcon icon(":/icons/invoice_icon.png");
    invoice_button = new QPushButton(icon, "Create new invoice", this);

    button_group_->addButton(invoice_button);
}

void Dashboard::button_connections() {
    connect(invoice_button, &QPushButton::clicked, this, [this]() { emit dash_navigation(Page::NewInvoice); });
}