#pragma once

#include "Domain/Accounts/Interfaces/IAccountManager.h"

namespace Application::Accounts {

// UI state only. ApiClient records a login here after the backend authenticates it.
// Database queries, password checks and account creation belong to the backend.
class FrontendAccountSession final : public Invoke::Domain::Accounts::IAccountManager {
  public:
    bool validEmail(const std::string& email) override;
    bool validName(const std::string& name) override;
    bool validatePassword(const std::string& password) override;
    bool doesAccountExist(const std::string& email) override;
    bool doesPasswordMatch(const std::string& password) override;
    bool needsAccountSetup(const std::string& email) override;
    void createAccount(std::string& email, std::string& password) override;
    void login(const std::string& email, const std::string& password) override;
    void logOut() override;
    bool is_logged_in() const override;
    std::shared_ptr<::User> getAccount() override;

  private:
    std::shared_ptr<::User> user_;
};

} // namespace Application::Accounts
