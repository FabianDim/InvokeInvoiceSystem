#pragma once
#include "pch.h"
#include "Application/Accounts/AccountManager.h" // Ensure this is included before using AccountManager
#include "Infrastructure/Database/Mongo/MongoDBDataManager.h"
#include "Domain/Accounts/User.h"

enum class BusinessStep {
	ENTER_ABN,
	ENTER_NAME,
	ENTER_PHONE,
	ENTER_ADDRESS,
	ENTER_ACN,
	CONFIRM,
	DONE
};

enum class AddressStep {
	COUNTRY,
	STATE,
	CITY,
	STREET,
	POST_CODE,
	DONE
};

class BusinessDetails {
	friend class MainMenu;
	struct business {
		struct Address {
			std::string country;
			std::string streetAddress;
			std::string city;
			std::string stateOrProvince;
			std::string postcode;
		};
		std::string ABN;
		std::string businessName;
		std::string businessPhone;
		Address businessAddress;
		std::string ACN;
	};
public:
    // Fix the constructor initializer list to initialize both dbManager and accountManager
    BusinessDetails(AccountManager& accManager, MongoDBDataManager& dbManager)
        : dbManager(dbManager), accountManager(accManager) {}
	enum BusinessStep currentLevel = BusinessStep::ENTER_ABN;
	enum AddressStep current = AddressStep::COUNTRY;
	std::string toLower(std::string text);

	bool getCountry();
	bool getState();
	bool getCity();
	bool getPostCode();
	bool getStreetAddress();
	bool addressInput();
	bool abnInput();
	bool nameInput();
	bool phoneInput();
	bool acnInput();
	bool confirmInfo();
	bool validateBusiness(std::string businessID);
	bool addUserExistingBusiness(std::string businessID);
	void collectBusinessInfo();
	void updateAccountRequirement(std::string email);
	std::string makeBusinessID();
	bsoncxx::document::value createBusinessDoc();
	bool createBusinessArrayUsers();
	void insertBusinessDoc(bsoncxx::document::value doc);
	
	std::shared_ptr<User> thisUser;

private:
	int thisBusID;
	business userBusiness;
	MongoDBDataManager& dbManager;
	AccountManager& accountManager;
};
