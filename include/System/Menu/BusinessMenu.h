#pragma once

#include "pch.h"
#include "ClientMenu.h"
#include "Accounts/UserBusiness/BusinessDetails.h"
#include "Accounts/UserBusiness/Clients/ClientDetails.h"
#include "StockMenu.h"

class BusinessMenu {
	friend class BusinessManager;

private:
	AppContext& appCtx;
	std::shared_ptr<User> currentUser;
	std::map<std::string, std::string> businessMap;
	BusinessDetails businessDetails;
	std::shared_ptr<ClientDetails> clientDetails;
	ClientMenu clientMenu;
	StockMenu stockMenu;
	MongoDBDataManager& dbManager; // moved to end
	int maxBusinesses = 3;

	// Internal helpers
	void chooseBusiness();
	std::map<std::string, std::string> retrieveUsersBusinessIDs();
	bool validateUserBusiness(const std::string businessID);
	bool createClient();
	size_t userBusinessCount();
	void addSelfToBusinessID();
	bool validateMaxBusinesses();
	bool manageStock();

public:
	BusinessMenu(AppContext& ctx)
		: appCtx(ctx),
		currentUser(ctx.accountMgr.getAccount()),
		businessDetails(ctx.accountMgr),
		clientDetails(std::make_shared<ClientDetails>(ctx.accountMgr, ctx.businessMgr)),
		clientMenu(ctx.cliManager, clientDetails),
		stockMenu(ctx.businessMgr, ctx.accountMgr, ctx.dbMgr),
		dbManager(ctx.dbMgr) {
	}

	void setBusiness(const std::string businessID);
	void displayBusMenu();
};
