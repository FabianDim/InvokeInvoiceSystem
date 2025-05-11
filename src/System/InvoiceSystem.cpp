#include "InvoiceSystem.h"



void InvoiceSystem::startSystem() {
	MainMenu mainMenu;
    AccountManager accountManager;

    mainMenu.displayMenu(accountManager);
}
