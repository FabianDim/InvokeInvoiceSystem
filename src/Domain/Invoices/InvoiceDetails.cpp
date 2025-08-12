#include <cctype>
#include "Domain/Invoices/InvoiceDetails.h"
#include "bsoncxx/exception/exception.hpp"
bool InvoiceDetails::enterInvoiceID() {
    std::cout << "Enter custom invoice ID or type auto for auto-generation: ";
    std::getline(std::cin >> std::ws, userInvoice.invoiceID);
    userInvoice.invoiceID = makeInvoiceID();
    if (toLower(userInvoice.invoiceID) == "auto") {
        userInvoice.clientInvoiceID = userInvoice.invoiceID;
        currentStep = InvoiceStep::ENTER_DATE;
        return true;
    }
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
    if (cliManager.chooseAClient()) { // does this set the client correcly?
        currentStep = InvoiceStep::ENTER_STOCK;
        userInvoice.client = cliManager.getCurClient();
        return true;
    }
    return false;
}

bool InvoiceDetails::selectStock() {
    try {
        std::cout << cliManager.getCurClient()->getClientID();
        auto stockMapOpt = stkMgr.stockMap();

        if (!stockMapOpt.has_value()) {
            throw std::runtime_error("Failed to create stock map.");
            return false;
        }
        std::cout << colourLime("\nAll Stock Items:\n");
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
            std::cout << "Please search a stock item by its keyword or name: ";
            std::cin >> search;
            if (searchMap.contains(search)) {
                std::cout << colourLime("Search Results ('Done' to continue, 'Search' to search again, 'Back' to go back):\n");
                int count = 0;
                std::string choice = "";
                std::unordered_map<int, std::string> choiceMap;
                for (auto& result : searchMap[search]) {
                    count++;
                    choiceMap[count] = result;
                    std::cout << count << ". " << colourYellow(stockMap[result]) << " | " << result << std::endl;
                }
                std::cout << "Enter a number from 1 - " << count << ": ";
                std::cin >> choice;

                if (toLower((choice)) == "search") {
                    continue;
                }
                else if (toLower((choice)) == "back") {
                    currentStep = InvoiceStep::ENTER_CLIENT;
                    return false;
                }
                else if (toLower((choice)) == "done") {
                    currentStep = InvoiceStep::ENTER_PAYMENT;
                    return true;
                }
                if ((choice.size() == 1) && isdigit(*choice.c_str()) && choiceMap.contains(stoi(choice))) {
                    stkMgr.setStockItem(choiceMap[stoi(choice)]);

                    userInvoice.stockQuantities[stkMgr.getCurrentStockItem()] = stockQuantity(); // Initialize stock quantity to 1
                }
            }
            else if(toLower(search) == "back") {
                currentStep = InvoiceStep::ENTER_CLIENT;
                return false;
            }
        } while (toLower(search) != "done");
    }
    catch (std::exception& e) {
        std::cerr << "Error selecting stock: " << e.what() << std::endl;
    }
    currentStep = InvoiceStep::CHOOSE_TEMPLATE;
    return true;
}

bool InvoiceDetails::chooseTemplate() {
    std::cout << "\nPlease choose a number of the template you want: \n";
    int templateChoice{};
    for (auto& templates : invoiceTemplate) {
        std::cout << templates.first << ". " << templates.second << " Template\n";
    }
    std::cout << "Choice: ";
    std::cin >> templateChoice;
    if (invoiceTemplate.find(templateChoice) != invoiceTemplate.end()) {
        userInvoice.invoiceTemplate = to_enum(invoiceTemplate.at(templateChoice));
        currentStep = InvoiceStep::ENTER_PAYMENT;
        return true;
    }
    else {
        std::cerr << "Invalid template choice." << std::endl;
        return false;
    }
    return false;
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
    std::cout << "Client ID: " << userInvoice.client->getClientID() << "\n";
    for (auto& [itemPtr, qty] : userInvoice.stockQuantities) {
        std::cout << "Stock: " << itemPtr->getStockID() << ", Quantity: " << qty << "\n";
    }
    std::cout << "Template: " << to_string(userInvoice.invoiceTemplate) << "\n";
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
        case InvoiceStep::CHOOSE_TEMPLATE:
            if (!chooseTemplate()) continue;
            break;
        case InvoiceStep::ENTER_PAYMENT:
            if (!enterPayment()) continue;
            break;
        case InvoiceStep::CONFIRM:
            if (!confirmInfo()) continue;
            break;
        case InvoiceStep::DONE:
            insertInvoiceDoc(createInvoiceDoc());
            setInvoiceToObject();
            return;
        default:
            std::cout << "Unexpected step" << std::endl;
            break;
        }
    }
}

std::string InvoiceDetails::makeInvoiceID() {
    try {
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
    catch (const mongocxx::exception& e) {
        std::cerr << "MongoDB exception: " << e.what() << std::endl;
    }
    catch (const bsoncxx::exception& e) {
        std::cerr << "BSON exception: " << e.what() << std::endl;
    }
    catch (const std::bad_alloc& e) {
        std::cerr << "Memory allocation exception: " << e.what() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "General exception: " << e.what() << std::endl;
    }
    catch (...) {
        std::cerr << "Invoice is breaking for some reason." << std::endl;
    }
    return "INV00000000";
}

bsoncxx::document::value InvoiceDetails::createInvoiceDoc() {
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::finalize;

    bsoncxx::builder::basic::array stockArray;
    for (auto& [stockItemPtr, qty] : userInvoice.stockQuantities) {
        bsoncxx::builder::basic::document entry;
        entry.append(kvp("StockID", stockItemPtr->getStockID()));
        entry.append(kvp("Quantity", qty));
        stockArray.append(entry);
    }

    return document{}
        << "InvoiceID" << userInvoice.invoiceID
        << "ClientInvoiceID" << userInvoice.clientInvoiceID
        << "ClientID" << userInvoice.client->getClientID()
        << "InvoiceDate" << userInvoice.invoiceDate
        << "DueDate" << userInvoice.dueDate
        << "IsPaid" << userInvoice.isPaid
        << "GSTIncluded" << userInvoice.gstIncluded
        << "InvoiceTemplate" << to_string(userInvoice.invoiceTemplate)
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

int InvoiceDetails::stockQuantity() {
    int amount{};
    std::cout << "Please enter amount of this item: ";
    std::cin >> amount;
    return amount;
}

bool InvoiceDetails::setInvoiceToObject() {
    auto newInvoice = std::make_shared<Invoice>();

    newInvoice->setCliInvoiceID(userInvoice.clientInvoiceID);
    newInvoice->setInvoiceID(userInvoice.invoiceID);
    newInvoice->setCurrentDate(userInvoice.invoiceDate);
    newInvoice->setDueDate(userInvoice.dueDate);
    newInvoice->setIsPaid(userInvoice.isPaid);
    newInvoice->setTemplate(userInvoice.invoiceTemplate);
    newInvoice->setTaxAmount(userInvoice.gstIncluded);
    newInvoice->setNotes(userInvoice.notes);

    auto clientPtr = std::make_shared<Client>();
    clientPtr->setClientID(userInvoice.client->getClientID());
    newInvoice->setClient(clientPtr);

    std::unordered_map<std::shared_ptr<StockItem>, int> stockMap;
    for (const auto& [itemPtr, qty] : userInvoice.stockQuantities) {
        auto stockItem = std::make_shared<StockItem>();
        stockItem->setStockID(itemPtr->getStockID());
        stockMap[stockItem] = qty;
    }
    newInvoice->setStockQuantityMap(stockMap);

    auto currentBiz = bizManager.getBusiness();
    if (!currentBiz) {
        std::cerr << "No current business set." << std::endl;
        return false;
    }
    newInvoice->setBusiness(currentBiz);
    InvoiceManager::setCurInvoice(newInvoice);
    return true;
}
