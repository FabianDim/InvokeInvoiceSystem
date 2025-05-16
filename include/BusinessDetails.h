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
			string country;
			string streetAddress;
			string city;
			string stateOrProvince;
			string postcode;
		};
		string ABN;
		string businessName;
		Address businessAddress;
		string ACN;
	};
	enum BusinessStep currentLevel = BusinessStep::ENTER_ABN;
	enum AddressStep current = AddressStep::COUNTRY;
	string toLower(string text);
	
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
	void updateAccountRequirement();
	bsoncxx::document::value createBusinessDoc();
	void insertBusinessDoc(bsoncxx::document::value doc);
private:
	business userBusiness;
	AccountManager accountManager;
	MongoDBDataManager dbManager;
	//const shared_ptr<User> thisUser = accountManager.getAccount();
};