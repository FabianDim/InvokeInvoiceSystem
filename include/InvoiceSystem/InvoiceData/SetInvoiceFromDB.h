#pragma once
#include "pch.h"
#include "Invoice.h"
class SetInvoice {
public:
	std::shared_ptr<Invoice> setInvoice(std::string invoiceID);
	SetInvoice(MongoDBDataManager& dbManager) : dbManager(dbManager) {};
private:
	MongoDBDataManager& dbManager;
};
