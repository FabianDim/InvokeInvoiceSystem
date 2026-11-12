#include "Infrastructure/Logging/Logging.h"
#include "Application/Accounts/AccountManager.h"
#include <iostream>
#include <iso646.h>
#include <cctype>
std::shared_ptr<User> AccountManager::currentUser = nullptr;

bool AccountManager::validEmail(const std::string& email) {
    for (const char& c : email) {
        if (std::isspace(c)) {
            std::cout << "Email cannot include spaces" << std::endl;
            return false;
        }
        if (std::isupper(c)) {
            hasUppers = true;
        }
    }
    return true;
}

bool AccountManager::validName(const std::string& name) {
    for (const char& c : name) {
        if (std::isdigit(c) or std::ispunct(c)) {
            std::cout << "Names cannot include numbers or punctuation" << std::endl;
            return false;
        }
    }
    return true;
}

bool AccountManager::doesAccountExist(const std::string& email) {
    auto document = dataManager.findOne("Users", make_document(kvp("UserEmail", email)));
    if (document) {
        std::cout << "An account with that email exists already. Try again: " << std::endl;
        return true;
    }
    return false;
}

bool AccountManager::doesPasswordMatch(const std::string& password) {
    return false;
}

bool AccountManager::validatePassword(const std::string& password) {
    bool isDigit = false, isUpper = false, isSpecial = false;
    for (char c : password) {
        if (std::isdigit(c)) {
            isDigit = true;
        } else if (std::isupper(c)) {
            isUpper = true;
        } else if (std::ispunct(c)) {
            isSpecial = true;
        }
    }
    if (isDigit && isUpper && isSpecial) {
        std::cout << "Password is valid." << std::endl;
    } else {
        std::cout << "Password must contain at least one digit, one uppercase letter, and one special character."
                  << std::endl;
    }
    return isDigit && isUpper && isSpecial;
}

void AccountManager::createAccount(std::string& userEmail, std::string& userPassword) {
    ::Infrastructure::Logging::Operation operation(::Infrastructure::Logging::auth(), "AccountManager.createAccount");

    while (true) {
        std::cout << "Please enter your new email (or * to cancel): ";
        if (hasUppers)
            std::transform(userEmail.begin(), userEmail.end(), userEmail.begin(), ::tolower);
        if (userEmail == "*") {
            std::cout << "Account creation canceled.\n";
            return;
        }
        if (!doesAccountExist(userEmail) && validEmail(userEmail)) {
            break;
        }
    }
    std::string storedHash;
    while (true) {
        std::cout << "Please enter your password: ";
        if (validatePassword(userPassword)) {
            storedHash = bcrypt::generateHash(userPassword);
            break;
        }
    }

    std::string firstName;
    std::string lastName;

    while (true) {
        std::cout << "Please enter your first name: ";
        std::getline(std::cin >> std::ws, firstName);
        if (firstName == "*") {
            std::cout << "Account creation canceled.\n";
            return;
        }
        std::cout << "Please enter your last name: ";
        std::getline(std::cin >> std::ws, lastName);
        if (lastName == "*") {
            std::cout << "Account creation canceled.\n";
            return;
        }
        if (validName(lastName) && validName(firstName)) {
            break;
        }
    }

    auto user = std::make_shared<User>(User(userEmail, userPassword));
    user->setUserEmail(userEmail);
    user->setPassword(storedHash);
    user->setFirstName(firstName);
    user->setLastName(lastName);

    auto builder = dataManager.buildNewUser(user);
    std::string id;
    if (!dataManager.insertDocument("Users", builder.view(), &id))
        throw std::runtime_error("Could not create account");
    user->setMongoUserID(id);

    accounts[userEmail] = std::move(user);
    currentUser = accounts[userEmail];
    std::cout << "Account created successfully!\n";
}

void AccountManager::login(const std::string& userEmail, const std::string& userPassword) {
    ::Infrastructure::Logging::Operation operation(::Infrastructure::Logging::auth(), "AccountManager.login");
    while (true) {
        if (dataManager.valid_password(userPassword, userEmail)) {
            SetUser setuser(dataManager);
            this->currentUser = setuser.setUserOnLogin(userEmail, userPassword);
            accounts[userEmail] = this->currentUser;
            return;
        } else {
            std::cout << "That username/password combo does not exist: try again.\n\n";
        }
    }
}

std::shared_ptr<User> AccountManager::getAccount() {
    return currentUser ? currentUser : nullptr;
}

bool AccountManager::is_logged_in() const {
    return currentUser != nullptr;
}

void AccountManager::logOut() {
    ::Infrastructure::Logging::Operation operation(::Infrastructure::Logging::auth(), "AccountManager.logOut");
    if (!is_logged_in()) {
        return;
    }
    currentUser = nullptr;
    std::cout << "Successfully logged out.\n";
}

void AccountManager::setTestUser(std::shared_ptr<User> user) {
    currentUser = user;
}

bool AccountManager::needsAccountSetup(const std::string& email) {
    auto document = dataManager.findOne("Users", make_document(kvp("UserEmail", email)));

    if (document) {
        auto view = document->view();
        auto element = dataManager.findElement("Users", view, "AccountSetupNeeded");
        if (element) {
            auto needsSetup{element->get_bool()};
            return needsSetup.value;
        }
    }
    return false;
}
