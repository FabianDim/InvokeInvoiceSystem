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

    LandingPage* landing_page();
    LoginPage* login_page();
    Dashboard* dashboard_page();
    InvoiceDetailsInput* new_invoice_page();
    NewInvoiceStock* new_invoice_stock_page();

  private:
    Ui::MainWindow* ui;

    /*App Setup*/

    QComboBox* pageComboBox;
    QStackedWidget* pagesStack;
    LandingPage* landingPage_ = nullptr;
    Dashboard* dashboard_page_ = nullptr;
    LoginPage* login_page_ = nullptr;
    InvoiceDetailsInput* new_invoice_page_ = nullptr;
    NewInvoiceStock* new_invoice_stock_ = nullptr;
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