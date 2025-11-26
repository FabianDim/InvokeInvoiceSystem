#pragma once
#include "pch.h"
#include "Application/Business/BusinessManager.h"
#include "Domain/Invoices/Invoice.h"
#include "Application/Invoices/SetInvoiceFromDB.h"

class InvoiceManager {
private:
	static std::shared_ptr<Invoice> currentInvoice;
	MongoDBDataManager& dbManager;
public:
	InvoiceManager(MongoDBDataManager& dbManager) : dbManager(dbManager) {}
	static std::shared_ptr<Invoice> getCurInvoice();
	static void setCurInvoice(std::shared_ptr<Invoice> invoice);
	//static void setCurInvoice(const std::string invoiceID);

};