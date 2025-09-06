#include "Application/Controllers/AppController.h"
#include "View/UICode/Views/LoginPage.h"

using namespace Application::Controllers;

AppController::AppController(App::Views::MainWindow* main,
                             Invoke::Domain::Accounts::IAccountManager& accountManager,
                             Infrastructure::Http::ApiClient* api,
                             QObject* parent)
    : main_(main), accountManager_(accountManager), api_(api) {
    auto* landing = main_->landing_page();
    QObject::connect(landing,                               // 1. Sender object (LandingPage instance)
                     &App::Views::LandingPage::navigate_to, // 2. Signal emitted by sender
                     main_,                                 // 3. Receiver object (MainWindow instance)
                     &App::Views::MainWindow::show_page     // 4. Slot (or method) to call on receiver
    );
    auto* login = main_->login_page();
    QObject::connect(login, &App::Views::LoginPage::login_requested, api_, &Infrastructure::Http::ApiClient::do_login);
}

void AppController::page_navigation(Page page) {
    main_->show_page(page);
    return;
}
