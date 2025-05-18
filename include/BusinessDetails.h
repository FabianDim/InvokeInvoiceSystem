#pragma once
#include "pch.h"
#include "AccountManager.h"
#include "MongoDBDataManager.h"
#include "User.h"

enum class BusinessStep {
	ENTER_ABN,
	ENTER_NAME,
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
		Address businessAddress;
		std::string ACN;
	};
public:
	BusinessDetails();
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
	bool acnInput();
	bool confirmInfo();
	void collectBusinessInfo();
	void updateAccountRequirement(std::string email);
	bsoncxx::document::value createBusinessDoc();
	void insertBusinessDoc(bsoncxx::document::value doc);
	std::shared_ptr<User> thisUser;

private:
	business userBusiness;
	MongoDBDataManager dbManager;
};
