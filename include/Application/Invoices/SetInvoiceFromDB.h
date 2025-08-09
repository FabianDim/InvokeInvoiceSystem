#pragma once
#include "pch.h"
#include "Domain/Invoices/Invoice.h"
#include "Application/Stock/SetStockFromDB.h"

class SetInvoice {
	friend class InvoiceManager;
public:
	SetInvoice(MongoDBDataManager& dbManager) : dbManager(dbManager) {};
private:
	std::shared_ptr<Invoice> setInvoice(const std::string invoiceID);
	MongoDBDataManager& dbManager;
	SetClient setClient;
};
