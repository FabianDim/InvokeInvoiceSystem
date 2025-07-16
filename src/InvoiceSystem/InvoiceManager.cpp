#include "InvoiceSystem/InvoiceData/InvoiceManager.h"
#include "InvoiceManager.h"

void InvoiceManager::setCurInvoice(std::shared_ptr<Invoice> invoice) {
	currentInvoice = invoice;
}

std::shared_ptr<Invoice> InvoiceManager::getCurInvoice() {
	return std::shared_ptr<Invoice>();
}

void InvoiceManager::setCurInvoice(const std::string invoiceID) {
	SetInvoice setInvoice;
	setInvoice.setInvoice(invoiceID);
}


