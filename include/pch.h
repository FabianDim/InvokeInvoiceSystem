// pch.h
#pragma once  

// Include standard headers  
#include <iostream>  
#include <string>  
#include <memory>  
#include <unordered_map>  
#include <optional>  
#include <functional>
#include <string_view>

// Include MongoDB headers  
#include <mongocxx/client.hpp>  
#include <mongocxx/instance.hpp>  
#include <mongocxx/uri.hpp>  
#include <bsoncxx/builder/stream/document.hpp>  
#include <bsoncxx/json.hpp>
#include <bsoncxx/stdx/string_view.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/types/value.hpp>
#include <bsoncxx/exception/exception.hpp>
#include <bsoncxx/builder/basic/kvp.hpp>  
#include "Infrastructure/Database/Mongo/MongoDBDataManager.h"
// Include bcrypt library  
#include "Infrastructure/Security/PasswordHashing/bcrypt.h"  

//include pdf table
extern "C" {
#include <hpdf.h>    
}
#include <cstring>
#include <cstdlib>
// Add any other necessary includes here
#include <jansson.h>
#include <vector>

#include <map>

#include <algorithm>
#include <thread>
#include <fmt/core.h>
#include <cctype>
#include <set>
#include <unordered_set>

//custom utils
#include "Utils/StringUtils.h"
#include "Utils/TemplateUtil.h"

//manager classes
#include "Application/Accounts/AccountManager.h"
#include "Application/Business/BusinessManager.h"
#include "Application/Business/Clients/ClientManager.h"   // if you have a ClientManager in that folder
#include "Application/Stock/StockManager.h"

//QT

#include <QString>
#include <QApplication>
#include <QMainWindow>
#include <QMessageBox>
#include <QInputDialog>
#include <QDebug>
#include <QFileDialog>
#include <QTableWidget>