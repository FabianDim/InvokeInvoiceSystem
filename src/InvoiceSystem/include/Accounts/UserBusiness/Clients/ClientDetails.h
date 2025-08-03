#pragma once
#include "pch.h"
#include "Accounts/User.h"
#include "Accounts/AccountManager.h"
#include "System/Database/MongoDBDataManager.h"
#include "Accounts/UserBusiness/BusinessRepository.h"
#include "Accounts/UserBusiness/BusinessManager.h"
//#include <Accounts/UserBusiness/Clients/ClientManager.h>

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

    struct clientData {
        std::string name;
        std::string phone;
        std::string email;
        address addr;
    };

    address currentAddress;
    clientData currentCli;

    ClientStep currentStep = ClientStep::ENTER_NAME;
    MongoDBDataManager dbManager;
    ClientAddressStep addressStep = ClientAddressStep::COUNTRY;
    BusinessManager& bizManager;
    AccountManager& accountManager;
    std::shared_ptr<BusinessRepository> curBusiness;
    int thisClientID = 0;

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
    bool addClientToBusiness();

public:
    ClientDetails(AccountManager& manager, BusinessManager& busManager)
        : accountManager(manager), bizManager(busManager) {};
    
    void collectClientInfo();
};
