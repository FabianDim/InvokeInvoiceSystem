#pragma once

#include "pch.h"
#include <QWidget>
#include <qlayout.h>
#include <QPushButton>
#include "Application/Accounts/AccountManager.h"
#include <QLineEdit>
#include <QCheckBox>

namespace App {
namespace Views {
class LoginPage : public QWidget {
    Q_OBJECT
  public:
    LoginPage(QWidget* parent = nullptr);
    ~LoginPage() = default;

  signals:
    void login_requested(const QString& email, const QString& password, bool remember);

  private:
    void createPageLayout();
    void on_login_clicked();
    QWidget* parent_widget_;
    QWidget* button_layout_;
    QWidget* form_layout_;
    QWidget* remember_me_layout_;
    QVBoxLayout* inner_layout_;

    QCheckBox* remember_me_;

    QLineEdit* email_input_;
    QLineEdit* password_input_;

    QPushButton* login_button_;
};
} // namespace Views
} // namespace App
