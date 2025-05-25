#include "InvoiceSystem.h"
#include "../../tests/InvoiceMenuTests.cpp"


void System::startSystem() {
    MainMenu mainMenu;
    AccountManager accountManager;
    mainMenu.isLoggedIn(accountManager);
}
