#include "Infrastructure/Database/Mongo/MongoDBHandler.h"
#include "pch.h"
#include <QtHttpServer>
#include "Infrastructure/Database/Services/AccountServices.h"
#include "Infrastructure/Database/Services/InvoiceServices.h"
#include "Domain/Accounts/Interfaces/IAccountManager.h"
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

class Server {

  private:
    QHttpServer httpServer_;
    QTcpServer* tcpServer_;
    int start_server();
    AccountServices account_services_;
    MongoDBDataManager& db_manager_;
    Invoke::Domain::Accounts::IAccountManager* account_manager_;

  public:
    Server(MongoDBDataManager& db_manager);
    ~Server() = default;
    void create_routes_basic();
    void create_routes_invoices();
    void create_routes_auth();
    void create_routes_business();
};