#pragma once
#include "pch.h"
#include "Domain/Invoices/Invoice.h"
#include "Application/Stock/SetStockFromDB.h"
#include "Application/Business/Clients/SetClientFromDB.h"
#include "Infrastructure/Database/Mongo/MongoDBDataManager.h"

class SetInvoice {
	friend class InvoiceManager;
public:
	SetInvoice(MongoDBDataManager& dbManager) : dbManager(dbManager) {};
private:
	std::shared_ptr<Invoice> setInvoice(const std::string invoiceID);
	MongoDBDataManager& dbManager;
	SetClient setClient;
};
