#pragma once
#include <pch.h>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>
#include <cstdlib> 
#include <mongocxx/exception/bulk_write_exception.hpp>
#include <string>
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

class MongoDBHandler {
	friend class MongoDBDataManager;
public:
	MongoDBHandler();
	mongocxx::database getDatabase() const;
private:
	static mongocxx::instance instance;
	mongocxx::uri uri;
	mongocxx::client client;
	std::string databaseName;

};
