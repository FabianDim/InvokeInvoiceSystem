#include "Application/Accounts/FrontendAccountSession.h"
#include "Domain/Accounts/User.h"
#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace Application::Accounts {

bool FrontendAccountSession::validEmail(const std::string& email) {
    return !email.empty() && std::none_of(email.begin(), email.end(), [](unsigned char character) {
        return std::isspace(character);
    });
}

bool FrontendAccountSession::validName(const std::string& name) {
    return !name.empty() && std::none_of(name.begin(), name.end(), [](unsigned char character) {
        return std::isdigit(character) || std::ispunct(character);
    });
}

bool FrontendAccountSession::validatePassword(const std::string& password) {
    bool digit = false, upper = false, special = false;
    for (unsigned char character : password) {
        digit = digit || std::isdigit(character);
        upper = upper || std::isupper(character);
        special = special || std::ispunct(character);
    }
    return digit && upper && special;
}

bool FrontendAccountSession::doesAccountExist(const std::string&) {
    throw std::logic_error("Account queries must be sent to the backend through ApiClient.");
}

bool FrontendAccountSession::doesPasswordMatch(const std::string&) {
    throw std::logic_error("Password checks must be sent to the backend through ApiClient.");
}

bool FrontendAccountSession::needsAccountSetup(const std::string&) {
    throw std::logic_error("Account queries must be sent to the backend through ApiClient.");
}

void FrontendAccountSession::createAccount(std::string&, std::string&) {
    throw std::logic_error("Account creation must be sent to the backend through ApiClient.");
}

void FrontendAccountSession::login(const std::string& email, const std::string&) {
    user_ = std::make_shared<User>(email, std::string{});
}

void FrontendAccountSession::logOut() {
    user_.reset();
}

bool FrontendAccountSession::is_logged_in() const {
    return user_ != nullptr;
}

std::shared_ptr<User> FrontendAccountSession::getAccount() {
    return user_;
}

} // namespace Application::Accounts
