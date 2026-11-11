#include "Infrastructure/Logging/Logging.h"
#include "View/InvokeInvoiceSystem.h"
#include <qfile.h>
#include <QIcon>
#include <Infrastructure/Http/FakeServer.h>
// int main() {
//
//	/*Main Executable*/
//     //InvokeInvoiceSystem iis;
//     //iis.run();
//
//	/*Invoice Menu*/
//     AccountManager accountManager;
//     BusinessManager businessManager;
//     MongoDBDataManager dbManager;
//     ClientManager cliManager(dbManager);
//     StockManager stkManager(dbManager,businessManager);
//
//
//     auto user = std::make_shared<User>("test@email.com", "password123");
//     user->setMongoUserID("USR00000001");
//
//     accountManager.setTestUser(user);
//     businessManager.setBusinessGlobally("BUS00000001");
//     BusinessRepository bizRepo;
//     // Create context
//
//     BusinessMenu businessMenu(accountManager, businessManager, cliManager, dbManager);
//     InvoiceMenu invoiceMenu(accountManager, businessManager,
//         bizRepo, dbManager, cliManager, stkManager);
//     invoiceMenu.setTestUser(user);
//     invoiceMenu.displayMenu();
//
//
//
//     //businessMenu.displayBusMenu();
//
//     return 0;
//
// }

// int main(int argc, char* argv[]) {
//	testing::InitGoogleTest(&argc, argv);
//	return RUN_ALL_TESTS();
// }

int main(int argc, char* argv[]) {

    MongoDBDataManager data_manager;
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Invoke");
    QCoreApplication::setApplicationName("InvokeInvoiceSystem");
    Infrastructure::Logging::initialize(true);
    qCInfo(Infrastructure::Logging::app) << "event=application.starting qt_version=" << qVersion();
    QObject::connect(&app, &QCoreApplication::aboutToQuit, []() {
        qCInfo(Infrastructure::Logging::app) << "event=application.stopping";
    });

    app.setWindowIcon(QIcon(":/icons/invoice_icon.png"));
    QFile f(":/styles/UI/Global.qss");
    if (f.open(QIODevice::ReadOnly)) {
        QString StyleSheet = QString::fromUtf8(f.readAll());
        app.setStyleSheet(StyleSheet);
    } else {
        qWarning() << "Failed to load stylesheet" << f.fileName() << f.errorString();
    }
    Invoke::Application::Auth::QSettingsSessionManager session_manager;


    AccountManager accountManager(data_manager);
    const QUrl backendUrl(qEnvironmentVariable("INVOICE_API_URL",
        "https://invoice-backend-app-69d26786220b.herokuapp.com"));
    const bool localBackend = backendUrl.scheme() == "http" && backendUrl.host() == "127.0.0.1";
    if (!backendUrl.isValid() || backendUrl.host().isEmpty() ||
        (backendUrl.scheme() != "https" && !localBackend) ||
        !backendUrl.userInfo().isEmpty() || backendUrl.hasQuery() || backendUrl.hasFragment() ||
        (!backendUrl.path().isEmpty() && backendUrl.path() != "/")) {
        qCritical() << "INVOICE_API_URL must be an HTTPS server URL (HTTP is allowed only for 127.0.0.1).";
        return EXIT_FAILURE;
    }
    std::unique_ptr<Server> localServer;
    if (localBackend && backendUrl.port() == 1234)
        localServer = std::make_unique<Server>(data_manager, &accountManager);
    qInfo() << "Invoice backend:" << backendUrl.toString();
    auto api = new Infrastructure::Http::ApiClient(backendUrl, &accountManager, &app);
    App::Views::MainWindow window(accountManager);
    Application::Controllers::AppController controller(&window, accountManager, api);
    window.resize(800, 800);
    window.show();
    qCInfo(Infrastructure::Logging::app) << "event=application.ready mode=" << (localBackend ? "local_backend" : "remote_backend");
    return app.exec();
}
