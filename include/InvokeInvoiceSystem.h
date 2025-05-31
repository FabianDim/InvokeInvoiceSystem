#pragma once

#include "pch.h"
#include "Accounts/User.h"
#include "System/InvoiceSystem.h"
#include "System/Menu/InvoiceMenu.h"

class InvokeInvoiceSystem {
    friend class MainMenu;
public:
    void run() {
        invoiceSystem.startSystem();
    }
private:
    System invoiceSystem;
};
