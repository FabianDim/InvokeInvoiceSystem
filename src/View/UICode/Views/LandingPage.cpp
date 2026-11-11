#include "View/UICode/Views/LandingPage.h"
#include "View/UiStyle.h"

namespace App::Views {
LandingPage::LandingPage(Invoke::Domain::Accounts::IAccountManager&, QWidget* parent) : QWidget(parent) {
    title_widget_ = UiStyle::page_content(this);
    button_layout_ = new QWidget(title_widget_);
    inner_layout_ = new QVBoxLayout(title_widget_);
    createPageLayout();
    connect(register_button_, &QPushButton::clicked, this, &LandingPage::on_register_clicked);
    connect(login_button_, &QPushButton::clicked, this, &LandingPage::on_login_clicked);
}

void LandingPage::createPageLayout() {
    inner_layout_->setContentsMargins(0, 0, 0, 0);
    inner_layout_->setSpacing(16);
    auto* title = UiStyle::label("Invoke Invoice System", title_widget_, "title");
    title->setObjectName("titleLabel");
    title->setAlignment(Qt::AlignCenter);
    title->setWordWrap(true);
    auto* subtitle = UiStyle::label("Create invoices and manage your business.", title_widget_, "subtitle");
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setWordWrap(true);
    inner_layout_->addWidget(title);
    inner_layout_->addWidget(subtitle);

    login_button_ = new QPushButton("Login", button_layout_);
    register_button_ = new QPushButton("Register", button_layout_);
    UiStyle::button(login_button_, "primary");
    UiStyle::button(register_button_);
    auto* buttons = new QHBoxLayout(button_layout_);
    buttons->setContentsMargins(0, 0, 0, 0);
    buttons->setSpacing(12);
    buttons->addWidget(login_button_, 1);
    buttons->addWidget(register_button_, 1);
    inner_layout_->addWidget(button_layout_);
    auto* offline = new QPushButton("Create an offline invoice", this);
    offline->setObjectName("offline_invoice_button");
    UiStyle::button(offline);
    inner_layout_->addWidget(offline);
    auto* demo_hint = UiStyle::label("Try it without an account. Demo records stay in memory; you can save the PDF.", this, "subtitle");
    demo_hint->setWordWrap(true);
    demo_hint->setAlignment(Qt::AlignCenter);
    inner_layout_->addWidget(demo_hint);
    connect(offline, &QPushButton::clicked, this, &LandingPage::offline_requested);
}

void LandingPage::on_register_clicked() { emit navigate_to(Page::Signup); }
void LandingPage::on_login_clicked() { emit navigate_to(Page::Login); }
} // namespace App::Views
