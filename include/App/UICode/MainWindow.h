#pragma once

#include <QMainWindow>
#include <QStackedWidget>

class IAccountManager;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }  // forward declaration from the .ui-generated header
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(IAccountManager& acctMgr, QWidget* parent = nullptr);
    ~MainWindow();

    

private:
    Ui::MainWindow* ui;

    /*App Setup*/

    QComboBox* pageComboBox;
    QStackedWidget* pagesStack;
	QWidget* landingPage;


    QMenu* fileMenu;
    QAction* newAct;
    QAction* openAct;
    QAction* saveAct;
    
    //Account related actions
    QMenu* accountMenu;
    QAction* logoutAct;
    QAction* loginAct;

    void createMenus();
    void createAccountActions();
    void createFileActions();

    IAccountManager& acctMgr;
};
