#include "InvoiceSystem.h"



void InvoiceSystem::startSystem() {
	MainMenu mainMenu;
    AccountManager accountManager;

    while (true) {
        mainMenu.displayMenu(accountManager);
    }
}
