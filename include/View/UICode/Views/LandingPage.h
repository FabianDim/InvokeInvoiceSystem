#pragma once

#include "pch.h"
#include <QWidget>
#include <qlayout.h>
#include <QPushButton>
#include "Application/Accounts/AccountManager.h"

class Invoke::Domain::Accounts::IAccountManager;
namespace App {
    namespace Views {
        class LandingPage : public QWidget {
            Q_OBJECT
          public:
            LandingPage(Invoke::Domain::Accounts::IAccountManager& account_manager, QWidget* parent = nullptr);
            ~LandingPage() = default;

          private:
            QWidget* createPageLayout();
            QWidget* title_widget_;
            QWidget* button_layout_;
            QVBoxLayout* inner_layout_;

            QPushButton* login_button_;
            QPushButton* register_button_;
        };
    } // namespace Views
} // namespace App
