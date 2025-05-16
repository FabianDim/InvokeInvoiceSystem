#include "BusinessDetails.h"

#include <cctype>

string BusinessDetails::toLower(string text) {
	transform(text.begin(), text.end(), text.begin(), ::tolower);
	return text;
}


bool BusinessDetails::addressInput() {
	while (true) {
		if (current == AddressStep::COUNTRY) {
			getCountry();
		}
		if (current == AddressStep::STATE) {
			getState();
		}
		if (current == AddressStep::CITY) {
			getCity();
		}
		if (current == AddressStep::POST_CODE) {
			getPostCode();
		}
		if (current == AddressStep::STREET) {
			getStreetAddress();
		}
	}
	currentLevel = BusinessStep::ENTER_ACN;
	return true;
}
bool BusinessDetails::getCountry() {
	cout << "Please enter your country*";
	string country;
	getline(std::cin >> std::ws, country);

	for (auto& c : country) {
		if (isdigit(c) or ispunct(c) || toLower(country) == "back") {
			return false;
		}
	}

	country = toLower(country);
	country[0] = toupper(country[0]);
	userBusiness.businessAddress.country = country;
	cout << endl;
	current = AddressStep::STATE;
	return true;
}

bool BusinessDetails::getState() {
	cout << "Please enter your state or province*";
	string state;
	getline(std::cin >> std::ws, state);

	for (auto& c : state) {
		if (isdigit(c) or ispunct(c) || toLower(state) == "back") {
			return false;
		}
	}

	state = toLower(state);
	state[0] = toupper(state[0]);
	userBusiness.businessAddress.stateOrProvince = state;
	cout << endl;
	current = AddressStep::CITY;
	return true;
}

bool BusinessDetails::getCity() {
	cout << "Please enter your city*";
	string city;
	getline(std::cin >> std::ws, city);

	for (auto& c : city) {
		if (isdigit(c) or ispunct(c) || toLower(city) == "back") {
			return false;
		}
	}

	city = toLower(city);
	city[0] = toupper(city[0]);
	userBusiness.businessAddress.city = city;
	cout << endl;
	current = AddressStep::POST_CODE;
	return true;
}

bool BusinessDetails::getPostCode() {
	cout << "Please enter your post/zip code*";
	string postcode;
	getline(std::cin >> std::ws, postcode);

	for (auto& c : postcode) {
		if (isdigit(c) || toLower(postcode) == "back") {
			return false;
		}
		toupper(c);
	}

	userBusiness.businessAddress.postcode = postcode;
	cout << endl;
	current = AddressStep::STREET;
	return true;
}

bool BusinessDetails::getStreetAddress() {
	cout << "Please enter your street address*";
	string streetAddress;
	getline(std::cin >> std::ws, streetAddress);

	for (auto& c : streetAddress) {
		if (toLower(streetAddress) == "back") {
			return false;
		}
	}

	userBusiness.businessAddress.streetAddress = streetAddress;
	cout << endl;
	current = AddressStep::STREET;
	return true;
}
bool BusinessDetails::abnInput() {
	cout << "Enter your 10-digit ABN (or type 'back' to cancel): ";
	string abn;
	cin >> abn;

	if (toLower(abn) == "back")
		return false; // the main loop can choose to restart or change the step

	if (abn.length() != 10 || !all_of(abn.begin(), abn.end(), ::isdigit)) {
		cout << "Invalid ABN. Please try again.\n";
		return false; // stays on this step
	}
	currentLevel = BusinessStep::ENTER_NAME;
	userBusiness.ABN = abn;
	return true; // continue to next step
}

bool BusinessDetails::nameInput() {
		cout << "Name";
		string name;
		cin >> name;

		if (toLower(name) == "back")currentLevel = BusinessStep::ENTER_ABN; return false;
		userBusiness.businessName = name;
		currentLevel = BusinessStep::ENTER_ADDRESS;
		//if back do somethi
}

bool BusinessDetails::acnInput() {
	cout << "Enter your 9-digit ACN (or type 'back' to cancel): ";
	string acn;
	cin >> acn;

	if (toLower(acn) == "back")
		return false; // the main loop can choose to restart or change the step

	if (acn.length() != 9 || !all_of(acn.begin(), acn.end(), ::isdigit)) {
		cout << "Invalid ABN. Please try again.\n";
		return false; // stays on this step
	}
	currentLevel = BusinessStep::CONFIRM;
	userBusiness.ACN = acn;
	return true; // continue to next step
}

bool BusinessDetails::confirmInfo() {
	cout << "Take a moment to verify your information: \n";

	cout << "ABN: " << userBusiness.ABN << endl;
	cout << "Business Name: " << userBusiness.businessName << endl;
	cout << "Address: " << userBusiness.businessAddress.streetAddress
		<< ", " << userBusiness.businessAddress.postcode << ", \n" <<
		userBusiness.businessAddress.city << ", " << userBusiness.businessAddress.stateOrProvince << ", "
		<< userBusiness.businessAddress.country << endl;
	cout << "ACN: " << userBusiness.ACN << endl;
	string input;
	while (true) {
		cout << "Type to redo: <ABN>, <Name>, <Address>, <ACN>. or <done> to finish setup";
		cin >> input;
		if (toLower(input) == "done") {
			currentLevel = BusinessStep::DONE;
			return true;
		}
		else if (toLower(input) == "abn") {
			currentLevel = BusinessStep::ENTER_ABN;
			return false;
		}
		else if (toLower(input) == "name") {
			currentLevel = BusinessStep::ENTER_NAME;
			return false;
		}
		else if (toLower(input) == "address") {
			currentLevel = BusinessStep::ENTER_ADDRESS;
			return false;
		}
		else if (toLower(input) == "acn") {
			currentLevel = BusinessStep::ENTER_ACN;
			return true;
		}
		else {
			cout << "Invalid input, try again";
		}
	}
	return false;
}
void BusinessDetails::collectBusinessInfo() {


	cout << "Lets get to know you and your business a bit better, fields marked with a * are mandatory.\n\n";
	cout << "This information will be autofilled on invoices. You can change values later. \n Type 'Back' to edit previous answers.";
	while (true) {
		if (currentLevel == BusinessStep::ENTER_ABN) {
			if (!abnInput()) continue;
		}
		else if (currentLevel == BusinessStep::ENTER_NAME) {
			if (!nameInput()) continue;
		}
		else if (currentLevel == BusinessStep::ENTER_ADDRESS) {
			if (!addressInput()) continue;
		}
		else if (currentLevel == BusinessStep::ENTER_ACN) {
			if (!acnInput()) continue;
		}
		else if (currentLevel == BusinessStep::CONFIRM) {
			if (confirmInfo()) return;
		}

	}
}