#pragma once

#include "pch.h"
#include "Accounts/User.h"
#include "System/InvoiceSystem.h"
#include "System/Menu/InvoiceMenu.h"
#include "System/Menu/BusinessMenu.h"
#include <QApplication>
#include "App/UICode/MainWindow.h"
#include <QMainWindow>

class InvokeInvoiceSystem {
    friend class MainMenu;
public:
    InvokeInvoiceSystem(){};
private:

    System invoiceSystem;
};
