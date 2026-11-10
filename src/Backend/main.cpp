#include "Application/Accounts/AccountManager.h"
#include "Infrastructure/Http/FakeServer.h"

#include <QCoreApplication>
#include <QDebug>
#include <cstdlib>
#include <exception>

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Invoke");
    QCoreApplication::setApplicationName("InvokeInvoiceBackend");

    try {
        MongoDBDataManager dataManager;
        AccountManager accountManager(dataManager);
        Server server(dataManager, &accountManager);
        if (!server.is_listening())
            return EXIT_FAILURE;
        return app.exec();
    } catch (const std::exception& error) {
        qCritical() << "Backend startup failed:" << error.what();
        return EXIT_FAILURE;
    }
}
