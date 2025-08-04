#pragma once

#include "pch.h"
#include "InvoiceSystem/Accounts/User.h"
#include "InvoiceSystem/System/InvoiceSystem.h"
#include "InvoiceSystem/System/Menu/InvoiceMenu.h"
#include "InvoiceSystem/System/Menu/BusinessMenu.h"

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
