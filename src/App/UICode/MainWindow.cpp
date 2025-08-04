#include "App/UICode/MainWindow.h"

#include <QMenuBar>
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
        fileMenu(nullptr),
        newAct(nullptr),
        openAct(nullptr),
        saveAct(nullptr),
        loginAct(nullptr),
        logoutAct(nullptr)
{
    createActions();
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
    fileMenu->addSeparator();
    fileMenu->addAction(loginAct);
    fileMenu->addAction(logoutAct);

}

void MainWindow::createActions(){
	loginAct = new QAction(tr("&Login"), this);
	
    logoutAct = new QAction(tr("&Logout"), this);
}
