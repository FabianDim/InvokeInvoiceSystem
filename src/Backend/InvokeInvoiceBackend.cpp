#include <QCoreApplication>
#include <QDebug>
#include "Application/Accounts/AccountManager.h"
#include "Infrastructure/Database/Mongo/MongoDBDataManager.h"
#include "Infrastructure/Http/FakeServer.h"

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Invoke");
    QCoreApplication::setApplicationName("InvokeInvoiceBackend");

    MongoDBDataManager data_manager;
    AccountManager account_manager(data_manager);
    Server server(data_manager, &account_manager);
    if (!server.start()) {
        qCritical().noquote() << "Could not start backend on 127.0.0.1:1234:" << server.error_string();
        return 1;
    }
    return app.exec();
}
