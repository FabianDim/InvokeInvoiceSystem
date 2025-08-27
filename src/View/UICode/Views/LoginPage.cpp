#include "View/UICode/Views/LoginPage.h"

using namespace App::Views;

LoginPage::LoginPage(Invoke::Domain::Accounts::IAccountManager& account_manager, QWidget* parent) {
    title_widget_ = new QWidget(this);
    button_layout_ = new QWidget(form_layout_);
    form_layout_ = new QWidget(title_widget_);
    inner_layout_ = new QVBoxLayout(title_widget_);

    create_page_layout();
}

QWidget* LoginPage::create_page_layout() {
    email_input_ = new QLineEdit(form_layout_);
    password_input_ = new QLineEdit(form_layout_);
    login_button_ = new QPushButton("Login", button_layout_);
    return title_widget_;
}