#include "Infrastructure/Logging/Logging.h"
#include <QApplication>
#include <QCommandLineParser>
#include "Infrastructure/Http/FakeServer.h"
#include <qfile.h>
#include <QIcon>
#include "Application/Accounts/FrontendAccountSession.h"
#include "Application/Controllers/AppController.h"
#include "Infrastructure/Http/ApiClient.h"
#include "View/MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Invoke");
    QCoreApplication::setApplicationName("InvokeInvoiceSystem");
    QCoreApplication::setApplicationVersion(QStringLiteral(INVOKE_APP_VERSION));
    QCommandLineParser commandLine;
    commandLine.addHelpOption();
    commandLine.addVersionOption();
    commandLine.process(app);
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
    MongoDBDataManager data_manager;
    Application::Accounts::FrontendAccountSession accountManager;
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
