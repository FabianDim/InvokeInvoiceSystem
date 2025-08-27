#pragma once

#include "pch.h"
#include <QWidget>
#include <qlayout.h>
#include <QPushButton>
#include "Application/Accounts/AccountManager.h"
#include <QLineEdit>

class Invoke::Domain::Accounts::IAccountManager;
namespace App {
    namespace Views {
        class LoginPage : public QWidget {
            Q_OBJECT
          public:
            LoginPage(Invoke::Domain::Accounts::IAccountManager& account_manager, QWidget* parent = nullptr);
            ~LoginPage() = default;

          private:
            QWidget* create_page_layout();
            QWidget* title_widget_;
            QWidget* button_layout_;
            QWidget* form_layout_;

            QVBoxLayout* inner_layout_;

            QLineEdit* email_input_;
            QLineEdit* password_input_;

            QPushButton* login_button_;
        };
    } // namespace Views
} // namespace App
