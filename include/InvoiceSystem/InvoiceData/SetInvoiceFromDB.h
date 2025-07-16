#pragma once
#include "pch.h"
#include "Invoice.h"
#include "Stock/SetStockFromDB.h"

class SetInvoice {
	friend class InvoiceManager;
public:
	SetInvoice(MongoDBDataManager& dbManager) : dbManager(dbManager) {};
private:
	std::shared_ptr<Invoice> setInvoice(std::string invoiceID);
	MongoDBDataManager& dbManager;
	SetClient setClient;
};
