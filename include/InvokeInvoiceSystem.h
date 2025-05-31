#pragma once

#include "pch.h"
>>>>>>> Stashed changes

class InvokeInvoiceSystem {
    friend class MainMenu;
public:
    void run() {
        invoiceSystem.startSystem();
    }
private:
    System invoiceSystem;
};
