#pragma once

#include "pch.h"
#include <iostream>
#include "AccountManager.h"
#include "User.h"
#include "MainMenu.h"
#include <InvokeInvoiceSystem.h>
#include "InvoiceSystem.h"

class InvokeInvoiceSystem {
    friend class MainMenu;
public:
    void run() {
        invoiceSystem.startSystem();
    }
private:
    InvoiceSystem invoiceSystem;
};
