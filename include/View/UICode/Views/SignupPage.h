#pragma once

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QJsonDocument>
#include "Infrastructure/Enums/RouteEnums.h"

namespace App::Views {
class SignupPage : public QWidget {
    Q_OBJECT
  public:
    explicit SignupPage(QWidget* parent = nullptr);

  public slots:
    void set_status(const QString& message);

  signals:
    void signup_requested(const QJsonDocument& details);
    void navigate_to(Page page);

  private:
    void create_page_layout();
    void submit_signup();

    QLineEdit* first_name_input_;
    QLineEdit* last_name_input_;
    QLineEdit* email_input_;
    QLineEdit* password_input_;
    QLineEdit* confirm_password_input_;
    QLabel* status_label_;
};
} // namespace App::Views
