#include "View/MainWindow.h"
#include "Domain/Accounts/Interfaces/IAccountManager.h"
#include "View/UICode/Views/LandingPage.h" // use forward slashes
#include <QMenuBar>
#include <QComboBox>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QAction>
#include <QIcon>
#include <QDebug>
#include <QWidget>
#include "Infrastructure/Enums/RouteEnums.h"

App::Views::MainWindow::MainWindow(Invoke::Domain::Accounts::IAccountManager& acctMgr, QWidget* parent)
    : QMainWindow(parent), fileMenu(nullptr), newAct(nullptr), openAct(nullptr), saveAct(nullptr), loginAct(nullptr),
      logoutAct(nullptr), acctMgr(acctMgr), landingPage_(new App::Views::LandingPage(acctMgr, this)),
      pagesStack(new QStackedWidget(this)) {

    auto* central = new QWidget(this);
    auto* vbox = new QVBoxLayout(central);
    vbox->setAlignment(Qt::AlignCenter);
    // vbox->setContentsMargins(0, 0, 0, 0);
    vbox->setSpacing(6);
    setCentralWidget(central);

    // Top control(s)
    // pageComboBox->addItem("Landing Page");
    // vbox->addWidget(pageComboBox);

    // Pages
    auto* business_invoice_choice = business_invoice_choice_page();
    auto* landing = landing_page();
    auto* invoice = new_invoice_page();

    // start page
    pagesStack->setCurrentWidget(business_invoice_choice);
    // start page

    vbox->addWidget(pagesStack, /*stretch*/ 1, Qt::AlignCenter);

    // Menus & actions
    createAccountActions();
    createFileActions();
    createMenus();
    // Window bits
    setWindowTitle("Invoke Invoice System");
}

App::Views::MainWindow::~MainWindow() = default;

void App::Views::MainWindow::createMenus() {
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);

    accountMenu = menuBar()->addMenu(tr("&Account"));
    if (acctMgr.is_logged_in()) {
        accountMenu->addAction(logoutAct);
    } else {
        accountMenu->addAction(loginAct);
    }
}

void App::Views::MainWindow::createAccountActions() {
    loginAct = new QAction(tr("&Login"), this);
    logoutAct = new QAction(tr("&Logout"), this);
}

void App::Views::MainWindow::createFileActions() {
    newAct = new QAction(tr("&New"), this);
    openAct = new QAction(tr("&Open"), this);
    saveAct = new QAction(tr("&Save"), this);

    connect(newAct, &QAction::triggered, this, []() { qDebug() << "New action triggered"; });
    connect(openAct, &QAction::triggered, this, []() { qDebug() << "Open action triggered"; });
    connect(saveAct, &QAction::triggered, this, []() { qDebug() << "Save action triggered"; });
}

// application could pass in the page instance instead of using a switch
void App::Views::MainWindow::show_page(QWidget* widget) {
    pagesStack->setCurrentWidget(widget);
}

App::Views::LandingPage* App::Views::MainWindow::landing_page() {
    if (!landingPage_) {
        landingPage_ = new App::Views::LandingPage(acctMgr, this);
        pagesStack->addWidget(landingPage_);
    }
    return landingPage_;
}

App::Views::LoginPage* App::Views::MainWindow::login_page() {
    if (!login_page_) {
        login_page_ = new App::Views::LoginPage(this);
        pagesStack->addWidget(login_page_);
    }
    return login_page_;
}

App::Views::Dashboard* App::Views::MainWindow::dashboard_page() {
    if (!dashboard_page_) {
        dashboard_page_ = new App::Views::Dashboard(this);
        pagesStack->addWidget(dashboard_page_);
    }
    return dashboard_page_;
}

App::Views::InvoiceDetailsInput* App::Views::MainWindow::new_invoice_page() {
    if (!new_invoice_page_) {
        new_invoice_page_ = new App::Views::InvoiceDetailsInput(this);
        pagesStack->addWidget(new_invoice_page_);
    }
    return new_invoice_page_;
}

App::Views::NewInvoiceStock* App::Views::MainWindow::new_invoice_stock_page() {
    if (!new_invoice_stock_) {
        new_invoice_stock_ = new App::Views::NewInvoiceStock(this);
        pagesStack->addWidget(new_invoice_stock_);
    }
    return new_invoice_stock_;
}

App::Views::BusinessInvoiceChoice* App::Views::MainWindow::business_invoice_choice_page() {
    if (!business_invoice_choice_) {
        business_invoice_choice_ = new App::Views::BusinessInvoiceChoice(this);
        pagesStack->addWidget(business_invoice_choice_);
    }
    return business_invoice_choice_;
}
