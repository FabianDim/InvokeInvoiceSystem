#pragma once

#include "pch.h"
#include "Accounts/User.h"
#include "System/InvoiceSystem.h"
#include "System/Menu/InvoiceMenu.h"
#include "System/Menu/BusinessMenu.h"

class InvokeInvoiceSystem {
    friend class MainMenu;
public:
    InvokeInvoiceSystem(AccountManager accountManager) {};
    void run() {
        invoiceSystem.startSystem();
    }
private:
    System invoiceSystem;
};
