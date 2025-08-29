#include "View/UICode/Views/LoginPage.h"

using namespace App::Views;

LoginPage::LoginPage(Invoke::Domain::Accounts::IAccountManager& account_manager, QWidget* parent) {
    parent_widget_ = new QWidget(this);
    form_layout_ = new QWidget(parent_widget_);
    button_layout_ = new QWidget(form_layout_);
    inner_layout_ = new QVBoxLayout(parent_widget_);

    createPageLayout();
}

void LoginPage::createPageLayout() {
    email_input_ = new QLineEdit(form_layout_);
    password_input_ = new QLineEdit(form_layout_);
    login_button_ = new QPushButton("Login", button_layout_);
    login_button_->setObjectName("login_button_");
    // email_input_->setPlaceholderText("Email");
    // password_input_->setPlaceholderText("Password");

    auto* form = new QHBoxLayout(form_layout_);

    form->addWidget(email_input_);
    form->addWidget(password_input_);
    form->setSpacing(6);

    auto* buttons = new QHBoxLayout(button_layout_);
    buttons->addWidget(login_button_);

    inner_layout_->addWidget(form_layout_);
    inner_layout_->addWidget(button_layout_);
    inner_layout_->setContentsMargins(24, 24, 24, 24);
    inner_layout_->setSpacing(16);
}