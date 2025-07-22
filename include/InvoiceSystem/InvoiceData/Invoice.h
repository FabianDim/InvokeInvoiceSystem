#pragma once
#include "pch.h"
#include "Accounts/UserBusiness/BusinessManager.h"
#include "Accounts/UserBusiness/BusinessRepository.h"
#include "Accounts/UserBusiness/Clients/ClientManager.h"
#include "Accounts/UserBusiness/Clients/Client.h"
#include "Stock/StockManager.h"
#include "Stock/ClientStockItem.h"
#include "Utils/DateUtil.h"
#include "Utils/TemplateUtil.h"
#include "InvoiceTemplateEnum.h"
class Invoice{
private:
	std::string invoiceID;
	std::string clientInvoiceID;
	std::shared_ptr<BusinessRepository> business;
	std::shared_ptr<Client> client;
	std::string currentDate;
	std::string dueDate;
	std::unordered_map<std::shared_ptr<StockItem>, int> stockQuantityMap; // stockID to quantity
	InvoiceTemplateEnum invoiceTemplate;
	float totalAmount = 0.0f;
	float taxAmount = 0.0f;
	float discountAmount = 0.0f;
	bool isPaid = false;
	bool gstIncluded = false;
	std::string notes;
	//std::string invoiceTemplate;

public:
	// invoiceID
	std::string getInvoiceID() const { return invoiceID; }
	void setInvoiceID(const std::string& id) { invoiceID = id; }

	// clientInvoiceID
	std::string getCliInvoiceID() const { return clientInvoiceID; }
	void setCliInvoiceID(const std::string& id) { clientInvoiceID = id; }

	// business
	std::shared_ptr<BusinessRepository> getBusiness() const { return business; }
	void setBusiness(std::shared_ptr<BusinessRepository> biz) { business = biz; }

	// client
	std::shared_ptr<Client> getClient() const { return client; }
	void setClient(std::shared_ptr<Client> cli) { client = cli; }

	// currentDate
	std::string getCurrentDate() const { return currentDate; }
	void setCurrentDate(const std::string& date) { currentDate = date; }

	// dueDate
	std::string getDueDate() const { return dueDate; }
	void setDueDate(const std::string& date) { dueDate = date; }

	// stockQuantityMap
	std::unordered_map<std::shared_ptr<StockItem>, int>& getStockQuantityMap() { return stockQuantityMap; }
	void setStockQuantityMap(const std::unordered_map<std::shared_ptr<StockItem>, int>& map) { stockQuantityMap = map; }

	// totalAmount
	float getTotalAmount() const { return totalAmount; }
	void setTotalAmount(float total) { totalAmount = total; }

	//// taxAmount
	//float getTaxAmount() const { return taxAmount; }
	//void setTaxAmount(float tax) { taxAmount = tax; }

	// Template
	InvoiceTemplateEnum getTemplate() const { return invoiceTemplate; }
	void setTemplate(InvoiceTemplateEnum invTemplate) { invoiceTemplate = invTemplate; }

	// UsingGST
	bool getTaxAmount() const { return gstIncluded; }
	void setTaxAmount(bool gst) { gstIncluded = gst; }

	// discountAmount
	float getDiscountAmount() const { return discountAmount; }
	void setDiscountAmount(float discount) { discountAmount = discount; }

	// isPaid
	bool getIsPaid() const { return isPaid; }
	void setIsPaid(bool paid) { isPaid = paid; }

	// notes
	std::string getNotes() const { return notes; }
	void setNotes(const std::string& n) { notes = n; }
};