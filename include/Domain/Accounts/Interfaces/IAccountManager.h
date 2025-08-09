#pragma once
#include <string>
#include <memory>

class User; // global ::User forward-decl to match your AccountManager

namespace Invoke::Domain::Accounts {

    class IAccountManager {
    public:
        virtual ~IAccountManager() = default;

        // validation / queries
        virtual bool validEmail(const std::string& email) = 0;
        virtual bool validName(const std::string& name) = 0;
        virtual bool doesAccountExist(const std::string& username) = 0;
        virtual bool doesPasswordMatch(const std::string& password) = 0;
        virtual bool validatePassword(const std::string& password) = 0;

        // actions
        virtual void createAccount(std::string& userEmail,
            std::string& userPassword) = 0;
        virtual void login() = 0;
        virtual void logOut() = 0;

        // state
        virtual bool isLoggedIn() const = 0;
        virtual bool needsAccountSetup(const std::string& email) = 0;

        // access
        virtual std::shared_ptr<::User> getAccount() = 0;
    };

} // namespace Invoke::Domain::Accounts
