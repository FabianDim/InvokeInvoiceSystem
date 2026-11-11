#pragma once
#include "Infrastructure/Database/Mongo/MongoDBHandler.h"
#include "pch.h"
#include <QtHttpServer>
#include "Infrastructure/Database/Services/AccountServices.h"
#include "Infrastructure/Database/Services/InvoiceServices.h"
#include "Infrastructure/Database/Services/BusinessService.h"
#include "Domain/Accounts/Interfaces/IAccountManager.h"
#include "Infrastructure/Http/AuthSessions.h"
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

class Server {

  private:
    QHttpServer httpServer_;
    bool listening_ = false;
    int start_server();
    MongoDBDataManager& db_manager_;
    Infrastructure::Http::AuthSessions sessions_;
    std::shared_ptr<Infrastructure::Http::AuthSessions::Session> session_for(const QHttpServerRequest& request);
    // Infrastructure::Services::BusinessService business_service_;

  public:
    Server(MongoDBDataManager& db_manager, Invoke::Domain::Accounts::IAccountManager* account_manager_);
    ~Server() = default;
    bool is_listening() const { return listening_; }
    void create_routes_basic();
    void create_routes_invoices();
    void create_routes_auth();
    void create_routes_business();
    void create_routes_data();
};
