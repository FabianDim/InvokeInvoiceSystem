#pragma once

#include "pch.h"

class Client;

class SetClient {
public:
	static std::shared_ptr<Client> setClientFromDB(const std::string& clientID);
};