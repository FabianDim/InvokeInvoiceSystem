#pragma once
#include "pch.h"
#include "Accounts/User.h"
#include "Accounts/AccountManager.h" // Ensure this is included before using AccountManager
#include "System/Database/MongoDBDataManager.h"

enum class ClientStep {
    ENTER_NAME,
    ENTER_PHONE,
    ENTER_EMAIL,
    ENTER_ADDRESS,
    CONFIRM,
    DONE
};

enum class ClientAddressStep {
    COUNTRY,
    STATE,
    CITY,
    STREET,
    POST_CODE,
    DONE
};

class ClientDetails {
private:
    struct address {
        std::string country;
        std::string streetAddress;
        std::string city;
        std::string stateOrProvince;
        std::string postcode;
    };

    struct client {
        std::string name;
        std::string phone;
        std::string email;
        address addr;
    };

    address currentAddress;
    client currentClient;

    ClientStep currentStep = ClientStep::ENTER_NAME;
    ClientAddressStep addressStep = ClientAddressStep::COUNTRY;

    AccountManager& accountManager;
    MongoDBDataManager dbManager;
    int thisClientID;

    // Input steps
    bool nameInput();
    bool phoneInput();
    bool emailInput();
    bool addressInput();

    // Address substeps
    bool getCountry();
    bool getState();
    bool getCity();
    bool getPostCode();
    bool getStreetAddress();

    // Confirm and finalize
    bool confirmInfo();
    std::string makeClientID();

    // MongoDB
    bsoncxx::document::value createClientDoc();
    void insertClientDoc(bsoncxx::document::value doc);

    // Utility
    //std::string toLower(std::string text);

public:
    ClientDetails(AccountManager& manager) : accountManager(manager) {};
    void collectClientInfo();
};
