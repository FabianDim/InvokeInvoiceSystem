#include "View/UICode/Views/LoginPage.h"
#include <QDebug>
using namespace App::Views;

LoginPage::LoginPage(QWidget* parent) {
    parent_widget_ = new QWidget(this);
    form_layout_ = new QWidget(parent_widget_);
    button_layout_ = new QWidget(form_layout_);
    remember_me_layout_ = new QWidget(form_layout_);
    inner_layout_ = new QVBoxLayout(parent_widget_);

    createPageLayout();
}

void LoginPage::createPageLayout() {
    email_input_ = new QLineEdit(form_layout_);
    password_input_ = new QLineEdit(form_layout_);
    login_button_ = new QPushButton("Login", button_layout_);
    login_button_->setObjectName("login_button_");
    email_input_->setPlaceholderText("Email");
    password_input_->setPlaceholderText("Password");
    password_input_->setEchoMode(QLineEdit::Password);

    connect(login_button_, &QPushButton::clicked, this, &LoginPage::on_login_clicked);

    connect(password_input_, &QLineEdit::returnPressed, login_button_, &QPushButton::click);

    auto* form = new QHBoxLayout(form_layout_);

    remember_me_ = new QCheckBox("Remember Me");
    form->addWidget(email_input_);
    form->addWidget(password_input_);

    form->setSpacing(6);

    auto* buttons = new QHBoxLayout(button_layout_);
    buttons->addWidget(login_button_);

    auto* remember = new QHBoxLayout(remember_me_layout_);
    remember->addWidget(remember_me_);

    inner_layout_->addWidget(form_layout_, 0, Qt::AlignCenter);
    inner_layout_->addWidget(remember_me_layout_);
    inner_layout_->addWidget(button_layout_);
}

void LoginPage::on_login_clicked() {
    QString email = email_input_->text();
    QString password = password_input_->text();

    const QString trimmed_email = email.trimmed();
    emit login_requested(trimmed_email, password, remember_me_->isChecked());

    password_input_->selectAll();
    qDebug() << "Login requested with email:" << trimmed_email << "and password length:" << password.length();
}