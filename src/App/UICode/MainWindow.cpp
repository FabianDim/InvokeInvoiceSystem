#include "App/UICode/MainWindow.h"

#include <QMenuBar>
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
        fileMenu(nullptr),
        newAct(nullptr),
        openAct(nullptr),
        saveAct(nullptr)
{
    createMenus();
    setWindowTitle("Invoke Invoice System");
	setWindowIcon(QIcon("./icons/invoice_icon.jpg"));

}

MainWindow::~MainWindow() {
}
void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
}