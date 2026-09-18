#include "View/UICode/Views/SignupPage.h"
#include "Infrastructure/Enums/RouteEnums.h"
#include <QFormLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVBoxLayout>
#include "View/UiStyle.h"

namespace App::Views {
SignupPage::SignupPage(QWidget* parent) : QWidget(parent) {
    create_page_layout();
}

void SignupPage::create_page_layout() {
    auto* content = UiStyle::page_content(this);
    auto* layout = new QVBoxLayout(content);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);

    auto* title = new QLabel("Create your account", this);
    title->setObjectName("titleLabel");
    title->setProperty("role", "title");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);

    auto* subtitle = new QLabel("Set up your workspace and start creating invoices.", this);
    subtitle->setObjectName("subtitleLabel");
    subtitle->setProperty("role", "subtitle");
    subtitle->setWordWrap(true);
    subtitle->setAlignment(Qt::AlignCenter);
    layout->addWidget(subtitle);

    auto* form = new QFormLayout;
    form->setSpacing(10);
    form->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    auto add_field = [form, this](const QString& label, QLineEdit*& target, bool password = false) {
        target = new QLineEdit(this);
        target->setObjectName("form_input");
        target->setProperty("role", "input");
        target->setPlaceholderText(label);
        if (password)
            target->setEchoMode(QLineEdit::Password);
        form->addRow(UiStyle::label(label, this), target);
    };
    add_field("First name", first_name_input_);
    add_field("Last name", last_name_input_);
    add_field("Email", email_input_);
    add_field("Password", password_input_, true);
    add_field("Confirm password", confirm_password_input_, true);
    layout->addLayout(form);

    auto* buttons = new QHBoxLayout;
    auto* create = new QPushButton("Create account", this);
    create->setObjectName("register_button_");
    auto* back = new QPushButton("Back to login", this);
    back->setObjectName("login_button_");
    UiStyle::button(create, "primary");
    UiStyle::button(back);
    buttons->setSpacing(12);
    buttons->addWidget(back, 1);
    buttons->addWidget(create, 1);
    layout->addLayout(buttons);

    status_label_ = new QLabel(this);
    status_label_->setProperty("role", "status");
    status_label_->setWordWrap(true);
    status_label_->setAlignment(Qt::AlignCenter);
    layout->addWidget(status_label_);

    connect(create, &QPushButton::clicked, this, &SignupPage::submit_signup);
    connect(back, &QPushButton::clicked, this, [this]() { emit navigate_to(Page::Login); });
}

void SignupPage::submit_signup() {
    const QString first = first_name_input_->text().trimmed();
    const QString last = last_name_input_->text().trimmed();
    const QString email = email_input_->text().trimmed();
    const QString password = password_input_->text();
    if (first.isEmpty() || last.isEmpty() || email.isEmpty() || password.isEmpty()) {
        set_status("Complete all fields to create your account.");
        return;
    }
    if (password != confirm_password_input_->text()) {
        set_status("Passwords do not match.");
        return;
    }
    emit signup_requested(
        QJsonDocument(QJsonObject{{"firstName", first}, {"lastName", last}, {"email", email}, {"password", password}}));
}

void SignupPage::set_status(const QString& message) {
    status_label_->setText(message);
}
} // namespace App::Views
