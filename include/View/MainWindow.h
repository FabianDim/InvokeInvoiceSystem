#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <qlayout.h>
#include <QComboBox>
#include "UICode/Views/LoginPage.h"
#include "UICode/Views/LandingPage.h"

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

          private:
            Ui::MainWindow* ui;

            /*App Setup*/

            QComboBox* pageComboBox;
            QStackedWidget* pagesStack;
            LandingPage* landingPage_;

            LoginPage* loginPage;

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