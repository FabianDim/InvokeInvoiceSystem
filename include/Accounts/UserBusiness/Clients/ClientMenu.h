#pragma once
#include "ClientManager.h"
#include "Accounts/UserBusiness/Clients/ClientDetails.h"
#include "Accounts/AccountManager.h"

class ClientMenu {
public:

    void displayClientMenu();
    void displayClientMap();

    

private:
    AccountManager& accountManager;
    ClientManager& clientManager;
    std::shared_ptr<ClientDetails> clientDetails;
};