#pragma once

#include "System/Database/MongoDBDataManager.h"
#include "Accounts/AccountManager.h"
#include "Accounts/UserBusiness/BusinessManager.h"
#include <string>
#include <bsoncxx/builder/stream/document.hpp>

enum class StockStep {
    ENTER_NAME,
    ENTER_QUANTITY,
    ENTER_PRICE,
    ENTER_MARGIN,
    ENTER_KEYWORDS,
    ENTER_UNIT,
    CONFIRM,
    DONE
};

class StockDetails {
private:
    struct StockInfo {
        std::string name;
        int quantity = 0;
        float price = 0.0f;
        float margin = 0.0f;
        bsoncxx::builder::basic::array keyWords;
        std::string unit; // the medium of the product. box back each.
    } currentStock;

    MongoDBDataManager dbManager;
    AccountManager& accountManager;
    StockStep currentStep = StockStep::ENTER_NAME;
    int thisStockID = 0;

    std::string toLower(std::string text);

    bool nameInput();
    bool quantityInput();
    bool priceInput();
    bool marginInput();
    bool keyWordsInput();
    bool unitInput();
    bool confirmInfo();
    std::string makeStockID();

    bsoncxx::document::value createStockDoc();
    void insertStockDoc(bsoncxx::document::value doc);

    bool insertStockIDToBiz();

    std::string removePunct();
public:
    StockDetails(AccountManager& manager);
    void collectStockInfo();
};