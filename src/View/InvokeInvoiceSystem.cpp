#include <QApplication>
#include <QFile>
#include <QIcon>
#include "Application/Accounts/FrontendAccountSession.h"
#include "Application/Controllers/AppController.h"
#include "Infrastructure/Http/ApiClient.h"
#include "View/MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Invoke");
    QCoreApplication::setApplicationName("InvokeInvoiceSystem");
    app.setWindowIcon(QIcon(":/icons/invoice_icon.png"));
    QFile stylesheet(":/styles/UI/Global.qss");
    if (stylesheet.open(QIODevice::ReadOnly))
        app.setStyleSheet(QString::fromUtf8(stylesheet.readAll()));
    else
        qWarning() << "Failed to load stylesheet" << stylesheet.errorString();

    // Authentication and MongoDB live in InvokeInvoiceBackend. This object
    // only holds the frontend's state after a successful HTTP login.
    Application::Accounts::FrontendAccountSession account_session;
    Infrastructure::Http::ApiClient api(QUrl("http://127.0.0.1:1234"), &account_session);
    App::Views::MainWindow window(account_session);
    Application::Controllers::AppController controller(&window, account_session, &api);
    window.resize(800, 800);
    window.show();
    return app.exec();
}
