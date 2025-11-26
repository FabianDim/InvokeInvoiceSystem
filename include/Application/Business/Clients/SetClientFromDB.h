#pragma once

#include "pch.h"
#include "Domain/Business/Clients/Client.h"

class SetClient {
public:
	static std::shared_ptr<Client> setClientFromDB(const std::string& clientID);
};