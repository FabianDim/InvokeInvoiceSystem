#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <qlayout.h>
#include <QComboBox>
#include "UICode/Views/LoginPage.h"
#include "UICode/Views/LandingPage.h"
#include "Infrastructure/Enums/RouteEnums.h"
#include "UICode/Views/Dashboard.h"
#include "UICode/Views/NewInvoiceCreation.h"
#include "UICode/Views/NewInvoiceStock.h"
#include "UICode/Views/BusinessInvoiceChoice.h"
#include "UICode/Views/ManagementForm.h"
#include "UICode/Views/SignupPage.h"
#include "UICode/Views/ItemsPage.h"
#include "UICode/Views/DefaultInvoiceSettings.h"

namespace Invoke {
namespace Domain {
namespace Accounts {
class IAccountManager;
}
} // namespace Domain
} // namespace Invoke
class LandingPage;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
} // namespace Ui
QT_END_NAMESPACE

namespace App {
namespace Views {
class MainWindow : public QMainWindow {
    Q_OBJECT
  public:
    explicit MainWindow(Invoke::Domain::Accounts::IAccountManager& acctMgr, QWidget* parent = nullptr);
    ~MainWindow();
    void show_page(QWidget* widget);
    void set_offline(bool offline);

    LandingPage* landing_page();
    LoginPage* login_page();
    SignupPage* signup_page();
    Dashboard* dashboard_page();
    InvoiceDetailsInput* new_invoice_page();
    NewInvoiceStock* new_invoice_stock_page();
    BusinessInvoiceChoice* business_invoice_choice_page();
    ManagementForm* client_page();
    ManagementForm* business_settings_page();
    ManagementForm* stock_settings_page();
    ManagementForm* account_settings_page();
    ItemsPage* items_page();
    DefaultInvoiceSettings* default_invoice_settings_page();

  signals:
    void logged_out();
    void offline_exit_requested();

  private:
    Ui::MainWindow* ui;
    bool offline_ = false;
    QWidget* offline_banner_ = nullptr;

    /*App Setup*/

    QComboBox* pageComboBox;
    QStackedWidget* pagesStack;
    LandingPage* landingPage_ = nullptr;
    Dashboard* dashboard_page_ = nullptr;
    LoginPage* login_page_ = nullptr;
    SignupPage* signup_page_ = nullptr;
    BusinessInvoiceChoice* business_invoice_choice_ = nullptr;
    InvoiceDetailsInput* new_invoice_page_ = nullptr;
    NewInvoiceStock* new_invoice_stock_ = nullptr;
    ManagementForm* client_page_ = nullptr;
    ManagementForm* business_settings_page_ = nullptr;
    ManagementForm* stock_settings_page_ = nullptr;
    ManagementForm* account_settings_page_ = nullptr;
    ItemsPage* items_page_ = nullptr;
    DefaultInvoiceSettings* default_invoice_settings_page_ = nullptr;
    QVBoxLayout* mainLayout();

    QMenu* fileMenu;
    QAction* newAct;
    QAction* openAct;
    QAction* saveAct;

    // Account related actions
    QMenu* accountMenu;
    QAction* logoutAct;
    QAction* loginAct;

    void createMenus();
    void createAccountActions();
    void createFileActions();

    Invoke::Domain::Accounts::IAccountManager& acctMgr;

    /*Landing page*/
};
} // namespace Views
} // namespace App
