#include "BusinessDetails.h"
#include "AccountManager.h"
#include <cctype>

BusinessDetails::BusinessDetails(AccountManager& accountManager) :accountManager(accountManager) // Initialize reference
{
}

std::string BusinessDetails::toLower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), ::tolower);
    return text;
}

bool BusinessDetails::addressInput() {
    std::cout << "Please enter your business address: \n";
    while (true) {
        if (current == AddressStep::COUNTRY) {
            if (!getCountry()) continue;
        }
        if (current == AddressStep::STATE) {
            if (!getState()) continue;
        }
        if (current == AddressStep::CITY) {
            if (!getCity()) continue;
        }
        if (current == AddressStep::POST_CODE) {
            if (!getPostCode()) continue;
        }
        if (current == AddressStep::STREET) {
            if (!getStreetAddress()) continue;
        }
        if (current == AddressStep::DONE) {
            currentLevel = BusinessStep::ENTER_ACN;
            break;
        }
    }
    return true;
}

bool BusinessDetails::getCountry() {
    std::cout << "Please enter your country*: ";
    std::string country;
    std::getline(std::cin >> std::ws, country);

    for (auto& c : country) {
        if (std::isdigit(c) or std::ispunct(c) || toLower(country) == "back") {
            return false;
        }
    }

    country = toLower(country);
    country[0] = std::toupper(country[0]);
    userBusiness.businessAddress.country = country;
    current = AddressStep::STATE;
    return true;
}

bool BusinessDetails::getState() {
    std::cout << "Please enter your state or province*: ";
    std::string state;
    std::getline(std::cin >> std::ws, state);

    for (auto& c : state) {
        if (std::isdigit(c) or std::ispunct(c) || toLower(state) == "back") {
            return false;
        }
    }

    state = toLower(state);
    state[0] = std::toupper(state[0]);
    userBusiness.businessAddress.stateOrProvince = state;
    current = AddressStep::CITY;
    return true;
}

bool BusinessDetails::getCity() {
    std::cout << "Please enter your city*: ";
    std::string city;
    std::getline(std::cin >> std::ws, city);

    for (auto& c : city) {
        if (std::isdigit(c) or std::ispunct(c) || toLower(city) == "back") {
            return false;
        }
    }

    city = toLower(city);
    city[0] = std::toupper(city[0]);
    userBusiness.businessAddress.city = city;
    current = AddressStep::POST_CODE;
    return true;
}

bool BusinessDetails::getPostCode() {
    std::cout << "Please enter your post/zip code*: ";
    std::string postcode;
    std::getline(std::cin >> std::ws, postcode);

    for (auto& c : postcode) {
        if (toLower(postcode) == "back" || std::isspace(c)) {
            return false;
        }

        c = std::toupper(c);
    }

    userBusiness.businessAddress.postcode = postcode;
    current = AddressStep::STREET;
    return true;
}

bool BusinessDetails::getStreetAddress() {
    std::cout << "Please enter your street address*: ";
    std::string streetAddress;
    std::getline(std::cin >> std::ws, streetAddress);

    for (auto& c : streetAddress) {
        if (toLower(streetAddress) == "back") {
            return false;
        }
    }

    userBusiness.businessAddress.streetAddress = streetAddress;
    current = AddressStep::DONE;
    return true;
}

bool BusinessDetails::abnInput() {
    std::cout << "Enter your 10-digit ABN (or type 'back' to cancel): ";
    std::string abn;
    std::cin >> abn;

    if (abn == "mongoid") {
        std::cout << "here is the mongoid: " << accountManager.getAccount()->getMongoUserID();
    }

    if (toLower(abn) == "back")
        return false;

    if (abn.length() != 10 || !std::all_of(abn.begin(), abn.end(), ::isdigit)) {
        std::cout << "Invalid ABN. Please try again.\n";
        return false;
    }
    currentLevel = BusinessStep::ENTER_NAME;
    userBusiness.ABN = abn;
    return true;
}

bool BusinessDetails::nameInput() {
    std::cout << "Please enter your business name: ";
    std::string name;

    std::getline(std::cin >> std::ws, name);

    if (toLower(name) == "back") {
        currentLevel = BusinessStep::ENTER_ABN;
        return false;
    }
    std::cout << std::endl;
    userBusiness.businessName = name;
    currentLevel = BusinessStep::ENTER_PHONE;
    return true;
}

bool BusinessDetails::phoneInput() {
    std::cout << "Please enter your main business phone number <(xxx)0000000000: ";
    std::string phone;

    std::getline(std::cin >> std::ws, phone);
    //https://github.com/google/libphonenumber for later implementation;
    if (toLower(phone) == "back") {
        currentLevel = BusinessStep::ENTER_NAME;
        return false;
    }

    std::cout << std::endl;
    userBusiness.businessPhone = phone;
    currentLevel = BusinessStep::ENTER_ADDRESS;
    return true;
}

bool BusinessDetails::acnInput() {
    std::cout << "Please enter your ACN (leave blank if not applicable): ";
    std::string acn;
    std::cin >> acn;

    if (toLower(acn) == "back") {
        return false;
    }
    currentLevel = BusinessStep::CONFIRM;
    userBusiness.ACN = acn;
    return true;
}

bool BusinessDetails::confirmInfo() {
    std::cout << "Take a moment to verify your information: \n";

    std::cout << "ABN: " << userBusiness.ABN << std::endl;
    std::cout << "Business Name: " << userBusiness.businessName << std::endl;
    std::cout << "Phone Number: " << userBusiness.businessPhone << std::endl;
    std::cout << "Address: " << userBusiness.businessAddress.streetAddress
        << ", " << userBusiness.businessAddress.postcode << ", " <<
        userBusiness.businessAddress.city << ", " << userBusiness.businessAddress.stateOrProvince << ", "
        << userBusiness.businessAddress.country << std::endl;
    std::cout << "ACN: " << userBusiness.ACN << std::endl;
    std::string input;
    while (true) {
        std::cout << "Type to redo: <ABN>, <Name>, <Phone>, <Address>, <ACN>. or <done> to finish setup: ";
        std::cin >> input;
        if (toLower(input) == "done") {
            currentLevel = BusinessStep::DONE;
            std::cout << std::endl;
            std::cout << std::endl;
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
        else if (toLower(input) == "phone") {
            currentLevel = BusinessStep::ENTER_PHONE;
            return false;
        }
        else if (toLower(input) == "address") {
            currentLevel = BusinessStep::ENTER_ADDRESS;
            return false;
        }
        else if (toLower(input) == "acn") {
            currentLevel = BusinessStep::ENTER_ACN;
            return false;
        }
        else {
            std::cout << "Invalid input, try again";
        }
    }
    return false;
}

void BusinessDetails::collectBusinessInfo() {
    while (true) {
        if (currentLevel == BusinessStep::ENTER_ABN) {
            if (!abnInput()) continue;
        }
        if (currentLevel == BusinessStep::ENTER_NAME) {
            if (!nameInput()) continue;
        }
        if (currentLevel == BusinessStep::ENTER_ADDRESS) {
            if (!addressInput()) continue;
        }
        if (currentLevel == BusinessStep::ENTER_PHONE) {
            if (!phoneInput()) continue;
        }
        if (currentLevel == BusinessStep::ENTER_ACN) {
            if (!acnInput()) continue;
        }
        if (currentLevel == BusinessStep::CONFIRM) {
            if (!confirmInfo()) continue;
        }
        if (currentLevel == BusinessStep::DONE) {
            insertBusinessDoc(createBusinessDoc());
            return;
        }
        
    }
}

void BusinessDetails::updateAccountRequirement(std::string email) {
    //AccountManager accountManager;
    if (accountManager.getAccount() != nullptr) {
        email = accountManager.getAccount()->getEmail();
    }
    else {
        std::cout << "I am meant to output an email: " << email;
    }
    

    try {
        if (email != "") {
            dbManager.updateDoc("Users", make_document(kvp("UserEmail", email)), make_document(kvp("$set", make_document(kvp("AccountSetupNeeded", false)))));
            return;
        }
        else {
            std::cout << "No email initialised";
        }
    }
    catch (mongocxx::exception e) {
        std::cerr << e.what() << std::endl;
    }
}

bsoncxx::document::value BusinessDetails::createBusinessDoc() {
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::finalize;
    std::string address = userBusiness.businessAddress.streetAddress + " " +
        userBusiness.businessAddress.postcode + ", " +
        userBusiness.businessAddress.city + ", " + 
        userBusiness.businessAddress.stateOrProvince + ", "+ 
        userBusiness.businessAddress.country;
    return document{}
        << "UserID" << accountManager.getAccount()->getMongoUserID()
        << "ABN" << userBusiness.ABN
        << "Phone" << userBusiness.businessPhone
        << "BusinessName" << userBusiness.businessName
        << "BusinessAddress" << address
        << "ACN" << userBusiness.ACN
        << finalize;
}

void BusinessDetails::insertBusinessDoc(bsoncxx::document::value doc) {
    //AccountManager accountManager;
    std::string email = "";
    if (accountManager.getAccount() != nullptr) {
         email = accountManager.getAccount()->getEmail();
    }
    else {
        std::cout << "I am meant to output an email in insert: " << email;
    }
    try {   
        dbManager.insertDocument("Business", doc);
        updateAccountRequirement(email);
    }
    catch (mongocxx::exception e) {
        std::cerr << e.what() << std::endl;
    }
}
