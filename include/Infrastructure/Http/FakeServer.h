#include "Infrastructure/Database/Mongo/MongoDBHandler.h"
#include "pch.h"
#include <QtHttpServer>

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

class Server {
    MongoDBHandler dbHandler;
    const mongocxx::database InvokeDB;
  private:
    QHttpServer httpServer_;
    QTcpServer* tcpServer_;
    int start_server();
    ~Server();
  public:
    Server(); 
    void create_routes_basic();
    void create_routes_auth();

};