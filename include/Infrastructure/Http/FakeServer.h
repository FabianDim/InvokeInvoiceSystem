#include "Infrastructure/Database/Mongo/MongoDBHandler.h"

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

class Server {
    MongoDBHandler dbHandler;
    const mongocxx::database InvokeDB;

  public:
    MongoDBDataManager() : InvokeDB{dbHandler.getDatabase()} {}

    void start_server();
};