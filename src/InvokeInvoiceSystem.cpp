#include "InvoiceSystem.h"
#include "AccountManager.h"
#include <iostream>

int main() {
    InvoiceSystem::printTitleBox();

    AccountManager accountManager;
    //std::cout << "Welcome to the Invoke Invoice System(tm)." << std::endl;
    accountManager.createAccount();

    return 0;
}