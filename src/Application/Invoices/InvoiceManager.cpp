#include "Application/Invoices/InvoiceManager.h"

std::shared_ptr<Invoice> InvoiceManager::currentInvoice = nullptr;

void InvoiceManager::setCurInvoice(std::shared_ptr<Invoice> invoice) {
	currentInvoice = invoice;
}

std::shared_ptr<Invoice> InvoiceManager::getCurInvoice() {
	return currentInvoice;
}

//void InvoiceManager::setCurInvoice(const std::string invoiceID) {
//	SetInvoice setInvoices(dbManager);
//	setInvoices.setInvoice(invoiceID);
//}


