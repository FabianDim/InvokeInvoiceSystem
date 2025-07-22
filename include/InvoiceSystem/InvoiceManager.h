#pragma once
#include "pch.h"
#include "Accounts/UserBusiness/BusinessManager.h"
#include "InvoiceSystem/InvoiceData/Invoice.h"
#include "InvoiceSystem/InvoiceData/SetInvoiceFromDB.h"

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