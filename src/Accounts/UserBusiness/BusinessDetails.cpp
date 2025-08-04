#include <cctype>
#include "InvoiceSystem/Accounts/UserBusiness/BusinessDetails.h"

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
        switch (current) {
        case AddressStep::COUNTRY:
            if (!getCountry()) continue;
            break;
        case AddressStep::STATE:
            if (!getState()) continue;
            break;
        case AddressStep::CITY:
            if (!getCity()) continue;
            break;
        case AddressStep::POST_CODE:
            if (!getPostCode()) continue;
            break;
        case AddressStep::STREET:
            if (!getStreetAddress()) continue;
            break;
        case AddressStep::DONE:
            currentLevel = BusinessStep::ENTER_ACN;
            return true;
        default:
            // Optionally handle error state
            break;
        }
    }
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
    std::cout << "Address: " << 
        userBusiness.businessAddress.streetAddress << ", " << 
        userBusiness.businessAddress.postcode << ", " <<
        userBusiness.businessAddress.city << ", " << 
        userBusiness.businessAddress.stateOrProvince << ", " << 
        userBusiness.businessAddress.country << std::endl;

    std::cout << "ACN: " << userBusiness.ACN << std::endl;
    std::string input;
    while (true) {
        std::cout << "Type to redo: <ABN>, <Name>, <Phone>, <Address>, <ACN>. or <Done> to finish setup: ";
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

bool BusinessDetails::validateBusiness(std::string businessID) {

    businessID[0] = toupper(businessID[0]);
    businessID[1] = toupper(businessID[1]);
    businessID[2] = toupper(businessID[2]);
    auto filter = dbManager.findOne("Business", make_document(kvp("BusinessID", businessID)));
    if (!filter) {
        return false;
    }
    return true;
}

bool BusinessDetails::addUserExistingBusiness(std::string businessID) {
    if (businessID.size() <= 3) {
        std::cout << colourRed("Business not found! Try again or type * to exit") << std::endl;
        return false;
    }
    businessID[0] = toupper(businessID[0]);
    businessID[1] = toupper(businessID[1]);
    businessID[2] = toupper(businessID[2]);
    try {
        auto filter = dbManager.findOne("Business", make_document(kvp("BusinessID", businessID)));
        if(!filter){
            std::cout << colourRed("Business not found! Try again or type * to exit") << std::endl;
            return false;
        }
        auto update = make_document(kvp("$push", make_document(kvp("UserID", accountManager.getAccount()->getMongoUserID()))));
        auto collection = dbManager.getCollection("Business");
        collection->update_one(filter->view(), update.view());
        //figure out whats going on here
        dbManager.getCollection("Users")->find_one_and_update(
            make_document(kvp("UserID", accountManager.getAccount()->getMongoUserID())).view(),
            make_document(kvp("$push", make_document(kvp("BusinessIDs", businessID)).view())));

        updateAccountRequirement(accountManager.getAccount()->getEmail());
        return true;

    }
    catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return false;
        }
    return false;
}

void BusinessDetails::collectBusinessInfo() {
    while (true) {
        switch (currentLevel) {
        case BusinessStep::ENTER_ABN:
            if (!abnInput()) continue;
            break;
        case BusinessStep::ENTER_NAME:
            if (!nameInput()) continue;
            break;
        case BusinessStep::ENTER_ADDRESS:
            if (!addressInput()) continue;
            break;
        case BusinessStep::ENTER_PHONE:
            if (!phoneInput()) continue;
            break;
        case BusinessStep::ENTER_ACN:
            if (!acnInput()) continue;
            break;
        case BusinessStep::CONFIRM:
            if (!confirmInfo()) continue;
            break;
        case BusinessStep::DONE:
            insertBusinessDoc(createBusinessDoc());
            return;
        default:
            std::cout << "Unexpected input" << std::endl;
            break;
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

std::string BusinessDetails::makeBusinessID() {
    auto bID = dbManager.findOne("counters",
        make_document(kvp("_id", make_document(
            kvp("db", "InvokeInvoiceSystem"),
            kvp("coll", "Business")
        ))));
    if (bID->view()["business_value"] && bID->view()["business_value"].type() == bsoncxx::type::k_int64) {
        thisBusID = static_cast<int>(bID->view()["business_value"].get_int64().value + 1);
        //std::cout << thisUserID;
    }
    else {
        std::cerr << "Something went wrong." << std::endl;
    }//make 8 digits
    std::string prefix = "BUS";
    if (std::to_string(thisBusID).size() < 8) {
        int zerosNeeded = 8 - std::to_string(thisBusID).size();

        prefix += std::string(zerosNeeded, '0') += std::to_string(thisBusID);

    }
    return prefix;
}

bsoncxx::document::value BusinessDetails::createBusinessDoc() {
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::finalize;
    using bsoncxx::builder::stream::open_array;
    using bsoncxx::builder::stream::close_array;
    bsoncxx::builder::basic::array UserIDs;
    
    //for now we will initially push just the first users ID and later be able to add user ID's.
    UserIDs.append(accountManager.getAccount()->getMongoUserID());
    
    if (accountManager.needsAccountSetup(accountManager.getAccount()->getEmail())) {
        bsoncxx::builder::basic::array businessIDs;
        businessIDs.append(makeBusinessID());
        dbManager.getCollection("Users")->find_one_and_update(make_document(kvp("UserID", accountManager.getAccount()->getMongoUserID())).view(),
        make_document(kvp("$addToSet", make_document(kvp("BusinessIDs", businessIDs)).view())));
    }
    else {
        SetUser setUser;
        setUser.addBusinessToUser(accountManager.getAccount()->getMongoUserID(), makeBusinessID());
    }
    std::string address = userBusiness.businessAddress.streetAddress + " " +
        userBusiness.businessAddress.postcode + ", " +
        userBusiness.businessAddress.city + ", " + 
        userBusiness.businessAddress.stateOrProvince + ", "+ 
        userBusiness.businessAddress.country;
    return document{}
    << "BusinessID" << makeBusinessID()
    << "UserID" << UserIDs
        << "ABN" << userBusiness.ABN
        << "Phone" << userBusiness.businessPhone
        << "BusinessName" << userBusiness.businessName
        << "BusinessAddress" << address
        << "ACN" << userBusiness.ACN
        << "ClientIDs" << open_array << close_array
        << "StockIDs" << open_array << close_array
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
