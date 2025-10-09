#pragma once
#include <qobject.h>
#include "View/MainWindow.h"
#include "Infrastructure/Http/ApiClient.h"
#include "Infrastructure/Enums/RouteEnums.h"
#include "View/UICode/Views/LoginPage.h"
#include "View/UICode/Views/Dashboard.h"
#include "View/UICode/Views/NewInvoiceCreation.h"

class IAccountManager;

namespace Application::Controllers {
class AppController : public QObject {
    Q_OBJECT
  public:
    AppController(App::Views::MainWindow* main,
                  Invoke::Domain::Accounts::IAccountManager& accountManager,
                  Infrastructure::Http::ApiClient* api,
                  QObject* parent = nullptr);

    LandingPage* landing_page;
    App::Views::LoginPage* login_page;
    App::Views::Dashboard* dashboard_page;
    App::Views::InvoiceDetailsInput* new_invoice_page;

  private:
    App::Views::MainWindow* main_;
    Invoke::Domain::Accounts::IAccountManager& accountManager_;
    Infrastructure::Http::ApiClient* api_;
    App::Views::LandingPage* landingPage_;
    App::Views::Dashboard* dashboard_page_;
    App::Views::LoginPage* login_page_;
    App::Views::InvoiceDetailsInput* new_invoice_page_;

  public slots:
    void page_navigation(Page page);
};
} // namespace Application::Controllers