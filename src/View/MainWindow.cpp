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

    loginPage = new LoginPage(acctMgr, landingPage_);

    pagesStack->addWidget(landingPage_);
    pagesStack->addWidget(loginPage);

    vbox->addWidget(pagesStack, /*stretch*/ 1, Qt::AlignCenter);

    // Menus & actions
    createAccountActions();
    createFileActions();
    createMenus();
    // Window bits
    setWindowTitle("Invoke Invoice System");
    QIcon icon(":/icons/invoice_icon.ico");
    qDebug() << "Icon is null?" << icon.isNull();
    setWindowIcon(icon);
}

App::Views::MainWindow::~MainWindow() = default;

void App::Views::MainWindow::createMenus() {
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);

    accountMenu = menuBar()->addMenu(tr("&Account"));
    if (acctMgr.isLoggedIn()) {
        accountMenu->addAction(logoutAct);
    } else {
        accountMenu->addAction(loginAct);
    }
}

void App::Views::MainWindow::createAccountActions() {
    loginAct = new QAction(tr("&Login"), this);
    logoutAct = new QAction(tr("&Logout"), this);

    connect(landingPage_, &LandingPage::login_requested, this, [this]() { pagesStack->setCurrentWidget(loginPage); });
}

void App::Views::MainWindow::createFileActions() {
    newAct = new QAction(tr("&New"), this);
    openAct = new QAction(tr("&Open"), this);
    saveAct = new QAction(tr("&Save"), this);

    connect(newAct, &QAction::triggered, this, []() { qDebug() << "New action triggered"; });
    connect(openAct, &QAction::triggered, this, []() { qDebug() << "Open action triggered"; });
    connect(saveAct, &QAction::triggered, this, []() { qDebug() << "Save action triggered"; });
}
