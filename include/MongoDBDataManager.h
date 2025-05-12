#pragma once  
#include "pch.h"  
#include <mongocxx/client.hpp>  
#include <mongocxx/instance.hpp>  
#include <mongocxx/uri.hpp>  
#include <bsoncxx/builder/basic/document.hpp>  
#include <bsoncxx/builder/basic/kvp.hpp>  
#include <User.h>
#include <MongoDBHandler.h>

using bsoncxx::builder::basic::kvp;  
using bsoncxx::builder::basic::make_document;  

class MongoDBDataManager {  
public:
	MongoDBDataManager();
   void writeNewUser(User* newUser );

private:  
	MongoDBHandler DBHandler
	mongocxx::database InvokeDB;

};
