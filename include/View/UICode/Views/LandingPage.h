#pragma once

#include "pch.h"
#include <QWidget>
#include <qlayout.h>
#include <QPushButton>
#include "Application/Accounts/AccountManager.h"
#include "View/MainWindow.h"
#include "Infrastructure/Enums/RouteEnums.h"

class Invoke::Domain::Accounts::IAccountManager;
namespace App::Views {
class LandingPage : public QWidget {
    Q_OBJECT
  public:
    LandingPage(Invoke::Domain::Accounts::IAccountManager& account_manager, QWidget* parent = nullptr);
    ~LandingPage() = default;

  private:
    void createPageLayout();
    QWidget* title_widget_;
    QWidget* button_layout_;
    QVBoxLayout* inner_layout_;

    QPushButton* login_button_;
    QPushButton* register_button_;

  signals:
    void navigate_to(Page page);

  private slots:
    void on_login_clicked();
    void on_register_clicked();
};
} // namespace App::Views
