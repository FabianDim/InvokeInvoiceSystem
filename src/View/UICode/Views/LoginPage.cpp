#include "View/UICode/Views/LoginPage.h"
#include <QDebug>
#include <QFormLayout>
#include <QLabel>
#include "View/UiStyle.h"
using namespace App::Views;

LoginPage::LoginPage(QWidget* parent) : QWidget(parent) {
    parent_widget_ = UiStyle::page_content(this);
    form_layout_ = new QWidget(parent_widget_);
    button_layout_ = new QWidget(form_layout_);
    remember_me_layout_ = new QWidget(form_layout_);
    inner_layout_ = new QVBoxLayout(parent_widget_);

    createPageLayout();
}

void LoginPage::createPageLayout() {
    inner_layout_->setContentsMargins(0, 0, 0, 0);
    inner_layout_->setSpacing(14);
    auto* title = new QLabel("Welcome back", parent_widget_);
    title->setObjectName("titleLabel");
    title->setProperty("role", "title");
    title->setAlignment(Qt::AlignCenter);
    auto* subtitle = new QLabel("Sign in to continue to your invoice workspace.", parent_widget_);
    subtitle->setObjectName("subtitleLabel");
    subtitle->setProperty("role", "subtitle");
    subtitle->setWordWrap(true);
    subtitle->setAlignment(Qt::AlignCenter);
    email_input_ = new QLineEdit(form_layout_);
    password_input_ = new QLineEdit(form_layout_);
    email_input_->setProperty("role", "input");
    password_input_->setProperty("role", "input");
    login_button_ = new QPushButton("Login", button_layout_);
    status_label_ = new QLabel(button_layout_);
    login_button_->setObjectName("login_button_");
    UiStyle::button(login_button_, "primary");
    status_label_->setProperty("role", "status");
    status_label_->setWordWrap(true);
    email_input_->setPlaceholderText("you@example.com");
    password_input_->setPlaceholderText("Your password");
    password_input_->setEchoMode(QLineEdit::Password);

    connect(login_button_, &QPushButton::clicked, this, &LoginPage::on_login_clicked);

    connect(password_input_, &QLineEdit::returnPressed, login_button_, &QPushButton::click);

    auto* form = new QFormLayout(form_layout_);
    form->setSpacing(10);
    form->setContentsMargins(0, 0, 0, 0);
    form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

    remember_me_ = new QCheckBox("Remember Me");
    form->addRow(UiStyle::label("Email", form_layout_), email_input_);
    form->addRow(UiStyle::label("Password", form_layout_), password_input_);

    auto* buttons = new QHBoxLayout(button_layout_);
    buttons->setContentsMargins(0, 0, 0, 0);
    buttons->setSpacing(12);
    buttons->addWidget(login_button_, 1);
    auto* signup_button = new QPushButton("Create an account", button_layout_);
    UiStyle::button(signup_button);
    buttons->addWidget(signup_button, 1);
    connect(signup_button, &QPushButton::clicked, this, [this]() { emit navigate_to(Page::Signup); });

    auto* remember = new QHBoxLayout(remember_me_layout_);
    remember->setContentsMargins(0, 0, 0, 0);
    remember->addWidget(remember_me_);

    inner_layout_->addWidget(title);
    inner_layout_->addWidget(subtitle);
    inner_layout_->addWidget(form_layout_);
    inner_layout_->addWidget(remember_me_layout_);
    inner_layout_->addWidget(button_layout_);
    inner_layout_->addWidget(status_label_);
}

void LoginPage::on_login_clicked() {
    QString email = email_input_->text();
    QString password = password_input_->text();
    const QString trimmed_email = email.trimmed();
    emit login_requested(trimmed_email, password, remember_me_->isChecked());

    password_input_->selectAll();
    qDebug() << "Login requested with email:" << trimmed_email << "and password length:" << password.length();
}

void LoginPage::set_status(const QString& message) {
    status_label_->setText(message);
}
