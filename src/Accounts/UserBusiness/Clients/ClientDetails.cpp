#include "Accounts/UserBusiness/Clients/ClientDetails.h"
#include <iostream>
#include <algorithm>

//ClientDetails::ClientDetails(AccountManager& manager) : accountManager(manager) {
//}

bool ClientDetails::nameInput() {
    std::cout << "Please enter your client's business name: ";
    std::string name;
    std::getline(std::cin >> std::ws, name);

    if (toLower(name) == "back") return false;

    currentClient.name = name;
    currentStep = ClientStep::ENTER_PHONE;
    return true;
}

bool ClientDetails::phoneInput() {
    std::cout << "Please enter your client's phone number: ";
    std::string phone;
    std::getline(std::cin >> std::ws, phone);

    if (toLower(phone) == "back") {
        currentStep = ClientStep::ENTER_NAME;
        return false;
    }

    currentClient.phone = phone;
    currentStep = ClientStep::ENTER_EMAIL;
    return true;
}

bool ClientDetails::emailInput() {
    std::cout << "Please enter your client's email address: ";
    std::string email;
    std::getline(std::cin >> std::ws, email);

    if (toLower(email) == "back") {
        currentStep = ClientStep::ENTER_PHONE;
        return false;
    }

    currentClient.email = email;
    currentStep = ClientStep::ENTER_ADDRESS;
    return true;
}

bool ClientDetails::addressInput() {
    while (true) {
        switch (addressStep) {
        case ClientAddressStep::COUNTRY:
            if (!getCountry()) continue;
            break;
        case ClientAddressStep::STATE:
            if (!getState()) continue;
            break;
        case ClientAddressStep::CITY:
            if (!getCity()) continue;
            break;
        case ClientAddressStep::POST_CODE:
            if (!getPostCode()) continue;
            break;
        case ClientAddressStep::STREET:
            if (!getStreetAddress()) continue;
            break;
        case ClientAddressStep::DONE:
            currentStep = ClientStep::CONFIRM;
            return true;
        default:
            break;
        }
    }
}

bool ClientDetails::getCountry() {
    std::cout << "Please enter country: ";
    std::string country;
    std::getline(std::cin >> std::ws, country);
    if (toLower(country) == "back") return false;
    currentClient.addr.country = country;
    addressStep = ClientAddressStep::STATE;
    return true;
}

bool ClientDetails::getState() {
    std::cout << "Please enter state or province: ";
    std::string state;
    std::getline(std::cin >> std::ws, state);
    if (toLower(state) == "back") return false;
    currentClient.addr.stateOrProvince = state;
    addressStep = ClientAddressStep::CITY;
    return true;
}

bool ClientDetails::getCity() {
    std::cout << "Please enter city: ";
    std::string city;
    std::getline(std::cin >> std::ws, city);
    if (toLower(city) == "back") return false;
    currentClient.addr.city = city;
    addressStep = ClientAddressStep::POST_CODE;
    return true;
}

bool ClientDetails::getPostCode() {
    std::cout << "Please enter postcode: ";
    std::string postcode;
    std::getline(std::cin >> std::ws, postcode);
    if (toLower(postcode) == "back") return false;
    currentClient.addr.postcode = postcode;
    addressStep = ClientAddressStep::STREET;
    return true;
}

bool ClientDetails::getStreetAddress() {
    std::cout << "Please enter street address: ";
    std::string street;
    std::getline(std::cin >> std::ws, street);
    if (toLower(street) == "back") return false;
    currentClient.addr.streetAddress = street;
    addressStep = ClientAddressStep::DONE;
    return true;
}

bool ClientDetails::confirmInfo() {
    std::cout << "Please confirm client info:\n";
    std::cout << "Name: " << currentClient.name << std::endl;
    std::cout << "Phone: " << currentClient.phone << std::endl;
    std::cout << "Email: " << currentClient.email << std::endl;
    std::cout << "Address: " << currentClient.addr.streetAddress << ", " << currentClient.addr.postcode << ", "
        << currentClient.addr.city << ", " << currentClient.addr.stateOrProvince << ", " << currentClient.addr.country << std::endl;
    std::string input;
    while (true) {
        std::cout << "Type to redo: <name>, <phone>, <email>, <address>. or <done> to finish setup: ";
        std::cin >> input;
        if (toLower(input) == "done") {
            currentStep = ClientStep::DONE;
            return true;
        }
        else if (toLower(input) == "name") {
            currentStep = ClientStep::ENTER_NAME;
            return false;
        }
        else if (toLower(input) == "phone") {
            currentStep = ClientStep::ENTER_PHONE;
            return false;
        }
        else if (toLower(input) == "email") {
            currentStep = ClientStep::ENTER_EMAIL;
            return false;
        }
        else if (toLower(input) == "address") {
            currentStep = ClientStep::ENTER_ADDRESS;
            addressStep = ClientAddressStep::COUNTRY;
            return false;
        }
        else {
            std::cout << "Invalid input, try again\n";
        }
    }
}

void ClientDetails::collectClientInfo() {
    while (true) {
        switch (currentStep) {
        case ClientStep::ENTER_NAME:
            if (!nameInput()) continue;
            break;
        case ClientStep::ENTER_PHONE:
            if (!phoneInput()) continue;
            break;
        case ClientStep::ENTER_EMAIL:
            if (!emailInput()) continue;
            break;
        case ClientStep::ENTER_ADDRESS:
            if (!addressInput()) continue;
            break;
        case ClientStep::CONFIRM:
            if (!confirmInfo()) continue;
            break;
        case ClientStep::DONE:
            insertClientDoc(createClientDoc());
            return;
        default:
            break;
        }
    }
}

std::string ClientDetails::makeClientID() {
    auto idDoc = dbManager.findOne("counters", make_document(kvp("_id", make_document(kvp("db", "InvokeInvoiceSystem"), kvp("coll", "Clients")))));
    if (idDoc && idDoc->view()["client_value"].type() == bsoncxx::type::k_int64) {
        thisClientID = static_cast<int>(idDoc->view()["client_value"].get_int64().value + 1);
    }
    else {
        std::cerr << "Error generating Client ID\n";
    }
    std::string prefix = "CLI";
    if (std::to_string(thisClientID).size() < 8) {
        int zerosNeeded = 8 - std::to_string(thisClientID).size();
        prefix += std::string(zerosNeeded, '0') + std::to_string(thisClientID);
    }
    return prefix;
}

bsoncxx::document::value ClientDetails::createClientDoc() {
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::finalize;

    std::string userID = accountManager.getAccount()->getMongoUserID();

    std::string address = currentClient.addr.streetAddress + " " + currentClient.addr.postcode + ", " +
        currentClient.addr.city + ", " + currentClient.addr.stateOrProvince + ", " + currentClient.addr.country;

    return document{}
        << "ClientID" << makeClientID()
        << "UserID" << userID
        << "ClientName" << currentClient.name
        << "Phone" << currentClient.phone
        << "Email" << currentClient.email
        << "Address" << address
        << finalize;
}

void ClientDetails::insertClientDoc(bsoncxx::document::value doc) {
    std::string email = accountManager.getAccount() ? accountManager.getAccount()->getEmail() : "";
    try {
        dbManager.insertDocument("Clients", doc);
    }
    catch (const mongocxx::exception& e) {
        std::cerr << e.what() << std::endl;
    }
}
