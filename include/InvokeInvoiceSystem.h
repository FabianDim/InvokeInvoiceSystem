#pragma once

#include "pch.h"

class InvokeInvoiceSystem {
    friend class MainMenu;
public:
    void run() {
        invoiceSystem.startSystem();
    }
private:
    System invoiceSystem;
};
