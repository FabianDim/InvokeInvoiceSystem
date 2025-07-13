#include "InvoiceSystem/InvoiceData/SetInvoiceFromDB.h"

std::shared_ptr<Invoice> SetInvoice::setInvoice(std::string invoiceID) {
	auto result = dbManager.findOne("Invoices", make_document(kvp("InvoiceID", invoiceID)));
	try {
		if (result) {
			auto view = result->view();
			auto invoice = std::make_shared<Invoice>(Invoice());
			

		}
	}
	catch (mongocxx::exception& e) {
		std::cout << e.what();
		return nullptr;
	}
	catch (bsoncxx::exception& e) {
		// this exception is thrown, when a invalid json is parsed.
		std::cout << "Tried to pare a wrong json: " << std::endl;
		//e.waht() is returning a human readable message which says withe a drawed arrow, where the error occured.
		std::cout << e.what() << std::endl;
		std::cout << "------------------------------------- end error message" << std::endl;
	}
}
