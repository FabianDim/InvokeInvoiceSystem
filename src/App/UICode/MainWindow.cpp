#include "App/UICode/MainWindow.h"
#include "Accounts/AccountServices/IAccountManager.h"

#include <QMenuBar>
MainWindow::MainWindow(IAccountManager& acctMgr, QWidget* parent)
    : QMainWindow(parent),
        fileMenu(nullptr),
        newAct(nullptr),
        openAct(nullptr),
        saveAct(nullptr),
        loginAct(nullptr),
        logoutAct(nullptr),
        acctMgr(acctMgr)
{
    createAccountActions();
    createFileActions();
    createMenus();
    setWindowTitle("Invoke Invoice System");
    QIcon icon(":/icons/invoice_icon.png");
    qDebug() << "Icon is null?" << icon.isNull();
    setWindowIcon(icon);
}

MainWindow::~MainWindow() {
}
void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    
    accountMenu = menuBar()->addMenu(tr("&Account"));
    if (acctMgr.isLoggedIn()) {
        accountMenu->addAction(logoutAct);
    }
    else {
        accountMenu->addAction(loginAct);
    }
}

void MainWindow::createAccountActions(){
	loginAct = new QAction(tr("&Login"), this);
    logoutAct = new QAction(tr("&Logout"), this);
}

void MainWindow::createFileActions(){
    newAct = new QAction(tr("&New"), this);
    openAct = new QAction(tr("&Open"), this);
    saveAct = new QAction(tr("&Save"), this);
    connect(newAct, &QAction::triggered, this, []() {
        qDebug() << "New action triggered";
    });
    connect(openAct, &QAction::triggered, this, []() {
        qDebug() << "Open action triggered";
    });
    connect(saveAct, &QAction::triggered, this, []() {
        qDebug() << "Save action triggered";
		});
}
