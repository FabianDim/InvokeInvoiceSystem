#include "View/UICode/Views/LandingPage.h"
#include <qlabel.h>
#include <QVBoxLayout>
#include <QPushButton>
#include "View/StyleSheet.h"
#include <QGraphicsDropShadowEffect>

namespace App::Views {

    LandingPage::LandingPage(Invoke::Domain::Accounts::IAccountManager& account_manager, QWidget* parent)
        : QWidget(parent) {
        title_widget_ = new QWidget(this);
        button_layout_ = new QWidget(title_widget_);
        inner_layout_ = new QVBoxLayout(title_widget_);
        setParent(parent);
        setWindowTitle("Landing Page");
        createPageLayout();
        // setFixedSize(800, 600);
        // wire the actions last

        connect(register_button_, &QPushButton::clicked, this, &LandingPage::on_register_clicked);
        connect(login_button_, &QPushButton::clicked, this, &LandingPage::on_login_clicked);
    }
    void App::Views::LandingPage::createPageLayout() {
        // Title
        auto* title_label = new QLabel(title_widget_);
        title_label->setTextFormat(Qt::RichText);
        title_label->setText(
            R"(<span style="color: white;">Welcome to the </span>
           <span style="color: #9c27b0; font-weight: 700;">Invoke Invoice System</span>)");
        title_label->setObjectName("titleLabel"); // matches Styles::widgetStyles key + QSS #titleLabel
        title_label->setAlignment(Qt::AlignCenter);

        // Buttons
        login_button_ = new QPushButton("Login", button_layout_);
        register_button_ = new QPushButton("Register", button_layout_);
        login_button_->setObjectName("login_button_");       // matches QSS: QPushButton#login_button_
        register_button_->setObjectName("register_button_"); // matches QSS: QPushButton#register_button_

        // Styles
        auto* effect = new QGraphicsDropShadowEffect(this);
        effect->setBlurRadius(16);
        effect->setOffset(0, 6);
        effect->setColor(QColor(0, 0, 0, 90));
        title_label->setGraphicsEffect(effect); // apply the drop shadow

        title_label->setStyleSheet(Styles::widgetStyles.at("titleLabel"));
        login_button_->setStyleSheet(Styles::widgetStyles.at("login_button_") + Styles::widgetStyles.at("button"));
        register_button_->setStyleSheet(Styles::widgetStyles.at("register_button_") +
                                        Styles::widgetStyles.at("button"));

        // Button row container + layout
        auto* buttons_hbox = new QHBoxLayout(button_layout_);
        buttons_hbox->setContentsMargins(0, 0, 0, 0);
        buttons_hbox->setSpacing(12);
        buttons_hbox->addWidget(login_button_);
        buttons_hbox->addWidget(register_button_);
        button_layout_->setLayout(buttons_hbox);

        // Stack everything vertically
        inner_layout_->setContentsMargins(24, 24, 24, 24);
        inner_layout_->setSpacing(16);
        inner_layout_->addWidget(title_label, 0, Qt::AlignCenter);
        inner_layout_->addWidget(button_layout_, 0, Qt::AlignCenter);

        setLayout(inner_layout_);
    }

    void LandingPage::on_register_clicked() {
        emit register_requested();
    }

    void LandingPage::on_login_clicked() {
        emit login_requested();
    }

} // namespace App::Views