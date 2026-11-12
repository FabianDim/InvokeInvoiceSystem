#include "Domain/Stock/StockDetails.h"
#include <iostream>
#include <algorithm>

StockDetails::StockDetails(AccountManager& manager) : accountManager(manager) {}

std::string StockDetails::toLower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), ::tolower);
    return text;
}

bool StockDetails::nameInput() {
    std::cout << "\nEnter stock name: ";
    std::getline(std::cin >> std::ws, currentStock.name);
    if (toLower(currentStock.name) == "back") return false;
    currentStep = StockStep::ENTER_QUANTITY;
    return true;
}

bool StockDetails::quantityInput() {
    std::cout << "Enter quantity on hand: ";
    std::string input;
    std::getline(std::cin >> std::ws, input);
    if (toLower(input) == "back") {
        currentStep = StockStep::ENTER_NAME;
        return false;
    }
    currentStock.quantity = std::stoi(input);
    currentStep = StockStep::ENTER_PRICE;
    return true;
}

bool StockDetails::priceInput() {
    std::cout << "Enter standard price: ";
    std::string input;
    std::getline(std::cin >> std::ws, input);
    if (toLower(input) == "back") {
        currentStep = StockStep::ENTER_QUANTITY;
        return false;
    }
    currentStock.price = std::stof(input);
    currentStep = StockStep::ENTER_MARGIN;
    return true;
}

bool StockDetails::marginInput() {
    std::cout << "Enter profit margin (0.2 = 20%): ";
    std::string input;
    std::getline(std::cin >> std::ws, input);
    if (toLower(input) == "back") {
        currentStep = StockStep::ENTER_PRICE;
        return false;
    }
    currentStock.margin = std::stof(input);
    currentStep = StockStep::ENTER_KEYWORDS;
    return true;
}

bool StockDetails::keyWordsInput() {
    std::cout << "Please enter keywords to facilitate searching for this stock ('Done' to continue) " << std::endl;
    
    std::string input;
    do{
        if (toLower(input) != "done") {
            std::getline(std::cin >> std::ws, input);
        }
        else {
            currentStep = StockStep::ENTER_UNIT;
            return true;
        }
        if (toLower(input) == "back") {
            currentStock.keyWords.clear();
            currentStep = StockStep::ENTER_MARGIN;
            return false;
        }
        currentStock.keyWords.append(toLower(input));
    } while (input != "done");
    currentStep = StockStep::ENTER_UNIT;
    return true;
}

bool StockDetails::unitInput() {
    std::cout << "Enter the type of unit e.g. box, bottle, each, pair: ";
    std::string input;
    std::getline(std::cin >> std::ws, input);
    if (toLower(input) == "back") {
        currentStep = StockStep::ENTER_KEYWORDS;
        return false;
    }
    currentStock.unit = input;
    currentStep = StockStep::CONFIRM;
    return true;
}

bool StockDetails::confirmInfo() {
    std::cout << "\nConfirm Stock Info:\n";
    std::cout << "Name: " << currentStock.name << "\n";
    std::cout << "Quantity: " << currentStock.quantity << "\n";
    std::cout << "Price: " << currentStock.price << "\n";
    std::cout << "Margin: " << currentStock.margin << "\n";
    std::string input;
    std::cout << "Type 'done' to save or 'back' to edit: ";
    std::cin >> input;
    if (toLower(input) == "done") {
        currentStep = StockStep::DONE;
        currentStock.keyWords.append(toLower(currentStock.name));

        //function to remove all punctuation from a string for keyword purposes.
        return true;
    }
    else {
        currentStep = StockStep::ENTER_NAME;
        return false;
    }
}

void StockDetails::collectStockInfo() {
    while (true) {
        switch (currentStep) {
        case StockStep::ENTER_NAME:
            if (!nameInput()) continue;
            break;
        case StockStep::ENTER_QUANTITY:
            if (!quantityInput()) continue;
            break;
        case StockStep::ENTER_PRICE:
            if (!priceInput()) continue;
            break;
        case StockStep::ENTER_MARGIN:
            if (!marginInput()) continue;
            break;
        case StockStep::ENTER_KEYWORDS:
            if (!keyWordsInput()) continue;
            break;
        case StockStep::ENTER_UNIT:
            if (!unitInput()) continue;
            break;
        case StockStep::CONFIRM:
            if (!confirmInfo()) continue;
            break;
        case StockStep::DONE: {
            insertStockDoc(createStockDoc());
            insertStockIDToBiz();
        }
        currentStep = StockStep::ENTER_NAME;
        return;
        default:
            break;
        }
    }
}

bsoncxx::document::value StockDetails::createStockDoc() {
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::finalize;
    using bsoncxx::builder::stream::open_array;
    using bsoncxx::builder::stream::close_array;
    BusinessManager bizManager;
    std::string userID = accountManager.getAccount()->getMongoUserID();
    return document{}
        << "BusinessID" << bsoncxx::oid(bizManager.getBusiness()->getBizID())
        << "Name" << currentStock.name 
        << "Quantity" << currentStock.quantity
        << "StdPrice" << currentStock.price
        << "ProfitMargin" << currentStock.margin
        << "ProductKeywords" << currentStock.keyWords
        << "Unit Type" << currentStock.unit
        << finalize;
}

void StockDetails::insertStockDoc(bsoncxx::document::value doc) {
    try {
        inserted_id_.clear();
        if (!dbManager.insertDocument("Stock", doc.view(), &inserted_id_))
            throw std::runtime_error("Could not save stock");
        return;
    }
    catch (const mongocxx::exception& e) {
        std::cerr << e.what() << std::endl;
        return;
    }
}

bool StockDetails::insertStockIDToBiz() {
    try {
        auto business = BusinessManager::getBusiness();
        auto filter = dbManager.findOne("Business", make_document(kvp("_id", bsoncxx::oid(business->getBizID()))));
        auto update = make_document(kvp("$addToSet", make_document(kvp("StockIDs", bsoncxx::oid(inserted_id_)))));
        auto result = dbManager.getCollection("Business")->find_one_and_update(filter->view(), update.view());

        return !result;
    }
    catch (const mongocxx::exception& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
    return false;
}