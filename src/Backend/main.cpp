#include "Infrastructure/Logging/Logging.h"
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
    QCoreApplication::setApplicationVersion(QStringLiteral(INVOKE_APP_VERSION));
    Infrastructure::Logging::initialize(false);
    qCInfo(Infrastructure::Logging::app) << "event=application.starting qt_version=" << qVersion();
    QObject::connect(&app, &QCoreApplication::aboutToQuit, []() {
        qCInfo(Infrastructure::Logging::app) << "event=application.stopping";
    });

    try {
        MongoDBDataManager dataManager;
        AccountManager accountManager(dataManager);
        Server server(dataManager, &accountManager);
        if (!server.is_listening())
            return EXIT_FAILURE;
        qCInfo(Infrastructure::Logging::app) << "event=application.ready mode=backend";
        return app.exec();
    } catch (const std::exception& error) {
        qCCritical(Infrastructure::Logging::app) << "event=application.startup_failed reason=exception";
        return EXIT_FAILURE;
    }
}
