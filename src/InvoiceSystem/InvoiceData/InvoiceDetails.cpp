#include <cctype>
#include "InvoiceSystem/InvoiceData/InvoiceDetails.h"
#include "InvoiceSystem/InvoiceData/Invoice.h"
#include "Accounts/UserBusiness/Clients/ClientManager.h"
#include "Accounts/UserBusiness/Clients/SetClientFromDB.h"
#include "Stock/StockManager.h"
#include "Utils/DateUtil.h"


std::string InvoiceDetails::toLower(std::string text) {
	std::transform(text.begin(), text.end(), text.begin(), ::tolower);
	return text;
}

bool InvoiceDetails::enterInvoiceID() {
	std::cout << "Enter custom invoice ID or leave blank for auto-generation: ";
	std::getline(std::cin >> std::ws, userInvoice.invoiceID);
	currentStep = InvoiceStep::ENTER_DATE;
	return true;
}

bool InvoiceDetails::enterDate() {
	std::cout << "Enter invoice date (YYYY-MM-DD): ";
	std::getline(std::cin >> std::ws, userInvoice.invoiceDate);
	currentStep = InvoiceStep::ENTER_DUE;
	return true;
}

bool InvoiceDetails::enterDueDate() {
	std::cout << "Enter due date (YYYY-MM-DD): ";
	std::getline(std::cin >> std::ws, userInvoice.dueDate);
	currentStep = InvoiceStep::ENTER_CLIENT;
	return true;
}

bool InvoiceDetails::selectClient() {
	cliManager.chooseAClient();
	currentStep = InvoiceStep::ENTER_STOCK;
	return true;
}

bool InvoiceDetails::selectStock() {  
    try {  
        auto stockMapOpt = stkMgr.stockMap();  

		if (!stockMapOpt.has_value()) {
			throw std::runtime_error("Failed to create stock map.");
			return false;
		}

		auto stockMap = stockMapOpt.value();
        for (auto& [id, name] : stockMap) {  
            std::cout << colourYellow(name) << " | " << id << std::endl;  
        }  
        auto searchMapOpt = stkMgr.createSearchMap();  
        if (!searchMapOpt.has_value()) {  
            throw std::runtime_error("Failed to create search map.");  
			return false;
		}  
        auto searchMap = searchMapOpt.value();  

        std::string search = "";  
        do {  
            std::cin >> search;  
            if (searchMap.contains(search)) {  
                std::cout << colourLime("Search Results ('Done' to continue, 'Search' to search again, 'Back' to go back):\n");  
                int count = 0;  
                int choice = 0;  
                std::unordered_map<int, std::string> choiceMap;  
                for (auto& result : searchMap[search]) {  
                    count++;  
                    choiceMap[count] = result;  
                    std::cout << count << ". " << colourYellow(stockMap[result]) << " | " << result << std::endl;  
                }  
                std::cout << "Enter a number from 1 - " << count << ": ";  
                std::cin >> choice;  

                if (toLower(std::to_string(choice)) == "search") {  
                    continue;  
                } else if (toLower(std::to_string(choice)) == "back") {  
                    currentStep = InvoiceStep::ENTER_CLIENT;  
                    return false;  
                }  
                if (isdigit(choice) && choiceMap.contains(choice)) {  
                    stkMgr.setStockItem(choiceMap[choice]);  
                    userInvoice.stockQuantities[stkMgr.getCurrentStockItem()] = 1; // Initialize stock quantity to 1  
                }  
            }  
        } while (toLower(search) != "done");  
    } catch (std::exception& e) {  
        std::cerr << "Error selecting stock: " << e.what() << std::endl;  
    }  

    return true;  
}

bool InvoiceDetails::enterPayment() {
	std::string input;
	std::cout << "Is the invoice paid? (yes/no): ";
	std::cin >> input;
	userInvoice.isPaid = toLower(input) == "yes";

	std::cout << "Include GST? (yes/no): ";
	std::cin >> input;
	userInvoice.gstIncluded = toLower(input) == "yes";

	std::cout << "Any notes? (optional): ";
	std::cin.ignore();
	std::getline(std::cin, userInvoice.notes);

	currentStep = InvoiceStep::CONFIRM;
	return true;
}

bool InvoiceDetails::confirmInfo() {
	std::cout << "\nInvoice Preview:\n";
	std::cout << "Invoice ID: " << (userInvoice.invoiceID.empty() ? "(auto-generated)" : userInvoice.invoiceID) << "\n";
	std::cout << "Date: " << userInvoice.invoiceDate << "\n";
	std::cout << "Due Date: " << userInvoice.dueDate << "\n";
	std::cout << "Client ID: " << userInvoice.client << "\n";
	for (auto& [id, qty] : userInvoice.stockQuantities) {
		std::cout << "Stock: " << id << ", Quantity: " << qty << "\n";
	}
	std::cout << "Paid: " << (userInvoice.isPaid ? "Yes" : "No") << "\n";
	std::cout << "GST: " << (userInvoice.gstIncluded ? "Yes" : "No") << "\n";
	std::cout << "Notes: " << userInvoice.notes << "\n";

	std::string confirm;
	std::cout << "Type 'done' to confirm or 'redo' to restart: ";
	std::cin >> confirm;
	if (toLower(confirm) == "done") {
		currentStep = InvoiceStep::DONE;
		return true;
	}
	return false;
}

void InvoiceDetails::collectInvoiceInfo() {
	std::cout << std::endl;
	while (true) {
		switch (currentStep) {
		case InvoiceStep::ENTER_INVOICE_ID:
			if (!enterInvoiceID()) continue;
			break;
		case InvoiceStep::ENTER_DATE:
			if (!enterDate()) continue;
			break;
		case InvoiceStep::ENTER_DUE:
			if (!enterDueDate()) continue;
			break;
		case InvoiceStep::ENTER_CLIENT:
			if (!selectClient()) continue;
			break;
		case InvoiceStep::ENTER_STOCK:
			if (!selectStock()) continue;
			break;
		case InvoiceStep::ENTER_PAYMENT:
			if (!enterPayment()) continue;
			break;
		case InvoiceStep::CONFIRM:
			if (!confirmInfo()) continue;
			break;
		case InvoiceStep::DONE:
			insertInvoiceDoc(createInvoiceDoc());
			setCurrentInvoice();
			return;
		default:
			std::cout << "Unexpected step" << std::endl;
			break;
		}
	}
}

std::string InvoiceDetails::makeInvoiceID() {
	auto invID = dbManager.findOne("counters", make_document(kvp("_id", make_document(kvp("db", "InvokeInvoiceSystem"), kvp("coll", "Invoices")))));
	if (invID && invID->view()["invoice_value"].type() == bsoncxx::type::k_int64) {
		thisInvoiceID = static_cast<int>(invID->view()["invoice_value"].get_int64().value + 1);
	}
	std::string prefix = "INV";
	std::string numStr = std::to_string(thisInvoiceID);
	if (numStr.size() < 8) {
		prefix += std::string(8 - numStr.size(), '0') + numStr;
	}
	return prefix;
}

bsoncxx::document::value InvoiceDetails::createInvoiceDoc() {
	using bsoncxx::builder::stream::document;
	using bsoncxx::builder::stream::finalize;
	using bsoncxx::builder::stream::open_document;
	using bsoncxx::builder::stream::close_document;
	using bsoncxx::builder::stream::open_array;
	using bsoncxx::builder::stream::close_array;

	bsoncxx::builder::basic::array stockArray;
	for (auto& [stockID, qty] : userInvoice.stockQuantities) {
		bsoncxx::builder::basic::document entry;
		entry.append(kvp("StockID", stockID->getStockID()));
		entry.append(kvp("Quantity", qty));
		stockArray.append(entry);
	} //Change this to use stock items.

	return document{}
		<< "InvoiceID" << (userInvoice.invoiceID.empty() ? makeInvoiceID() : userInvoice.invoiceID)
		<< "ClientID" << userInvoice.client->getClientID()
		<< "InvoiceDate" << userInvoice.invoiceDate
		<< "DueDate" << userInvoice.dueDate
		<< "IsPaid" << userInvoice.isPaid
		<< "GSTIncluded" << userInvoice.gstIncluded
		<< "Notes" << userInvoice.notes
		<< "StockItems" << stockArray
		<< finalize;
}

void InvoiceDetails::insertInvoiceDoc(bsoncxx::document::value doc) {
	try {
		dbManager.insertDocument("Invoices", doc);
	}
	catch (const mongocxx::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}

bool InvoiceDetails::setCurrentInvoice() {
	std::shared_ptr<Invoice> newInvoice = std::make_shared<Invoice>();

	// Use custom ID if provided, else generate one
	if (!userInvoice.invoiceID.empty()) {
		newInvoice->setInvoiceID(userInvoice.invoiceID);
	}
	else {
		newInvoice->setInvoiceID(makeInvoiceID());
	}

	newInvoice->setCurrentDate(userInvoice.invoiceDate);
	newInvoice->setDueDate(userInvoice.dueDate);
	newInvoice->setIsPaid(userInvoice.isPaid);
	newInvoice->setTaxAmount(userInvoice.gstIncluded);
	newInvoice->setNotes(userInvoice.notes);

	// client
	auto client = std::make_shared<Client>();
	client->setClientID(userInvoice.client->getClientID());
	newInvoice->setClient(client);

	// stock map setup
	std::unordered_map<std::shared_ptr<StockItem>, int> stockMap;
	for (const auto& pair : userInvoice.stockQuantities) {
		auto stockItem = std::make_shared<StockItem>();
		stockItem->setStockID(pair.first->getStockID());
		stockMap[stockItem] = pair.second;
	}
	newInvoice->setStockQuantityMap(stockMap);

	// business
	auto currentBiz = BusinessManager::getBusiness();
	if (!currentBiz) {
		std::cerr << "No current business set." << std::endl;
		return false;
	}
	newInvoice->setBusiness(currentBiz);

	// If all fields are set, return true
	return true;
}
