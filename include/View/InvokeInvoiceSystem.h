#pragma once

#include "pch.h"
#include "Domain/Accounts/User.h"
#include "View/LegacyMenus/InvoiceSystem.h"
#include "View/LegacyMenus/InvoiceMenu.h"
#include "View/LegacyMenus/BusinessMenu.h"
#include <QApplication>
#include "View/MainWindow.h"
#include <QMainWindow>

class InvokeInvoiceSystem {
    friend class MainMenu;
public:
    InvokeInvoiceSystem(){};
private:

    System invoiceSystem;
};
