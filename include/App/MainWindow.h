#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <qlayout.h>

namespace Invoke {
    namespace Domain {
        namespace Accounts {
            class IAccountManager;  // correct forward declaration
        }
    }
}
class LandingPage;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }  // forward declaration from the .ui-generated header
QT_END_NAMESPACE


namespace App {
    namespace Views {
        class MainWindow : public QMainWindow {
            Q_OBJECT
        public:
            explicit MainWindow(Invoke::Domain::Accounts::IAccountManager & acctMgr, QWidget* parent = nullptr);
            ~MainWindow();



        private:
            Ui::MainWindow* ui;

            /*App Setup*/

            QComboBox* pageComboBox;
            QStackedWidget* pagesStack;
            QWidget* landingPage;
            QVBoxLayout* mainLayout();

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
            Invoke::Domain::Accounts::IAccountManager& acctMgr;

            /*Landing page*/
        };
    }
}