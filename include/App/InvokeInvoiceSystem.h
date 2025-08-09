#pragma once

#include "pch.h"
#include "Domain/Accounts/User.h"
#include "App/LegacyMenus/InvoiceSystem.h"
#include "App/LegacyMenus/InvoiceMenu.h"
#include "App/LegacyMenus/BusinessMenu.h"
#include <QApplication>
#include "App/MainWindow.h"
#include <QMainWindow>

class InvokeInvoiceSystem {
    friend class MainMenu;
public:
    InvokeInvoiceSystem(){};
private:

    System invoiceSystem;
};
