#pragma once

#include "pch.h"
#include <QWidget>
#include <qlayout.h>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include "Infrastructure/Enums/RouteEnums.h"

class QLabel;

namespace App {
namespace Views {
class LoginPage : public QWidget {
    Q_OBJECT
  public:
    LoginPage(QWidget* parent = nullptr);
    ~LoginPage() = default;

  public slots:
    void set_status(const QString& message);

  signals:
    void offline_requested();
    void login_requested(const QString& email, const QString& password, bool remember);
    void navigate_to(Page page);

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
    QLabel* status_label_;
};
} // namespace Views
} // namespace App
