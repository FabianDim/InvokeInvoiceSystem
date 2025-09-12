#include "Application/Controllers/AppController.h"
#include "View/UICode/Views/LoginPage.h"
#include "View/UICode/Views/LandingPage.h"
#include "Enums/RouteEnums.h"

using namespace Application::Controllers;

AppController::AppController(App::Views::MainWindow* main,
                             Invoke::Domain::Accounts::IAccountManager& accountManager,
                             Infrastructure::Http::ApiClient* api,
                             QObject* parent)
    : QObject(parent), main_(main), accountManager_(accountManager), api_(api) {
    QObject::connect(
        main_->landing_page(), &App::Views::LandingPage::navigate_to, this, &AppController::page_navigation);
    QObject::connect(
        main_->login_page(), &App::Views::LoginPage::login_requested, api_, &Infrastructure::Http::ApiClient::do_login);

    QObject::connect(
        main_->dashboard_page(), &App::Views::Dashboard::dash_navigation, this, &AppController::page_navigation);
}

void AppController::page_navigation(Page page) {
    switch (page) {
    case Page::Landing:
        main_->show_page(main_->landing_page());
        break;
    case Page::Login:
        main_->show_page(main_->login_page());
        break;
    case Page::Dashboard:
        main_->show_page(main_->dashboard_page());
        break;
    case Page::NewInvoice:
        main_->show_page(main_->new_invoice_page());
        break;
    default:
        break;
    }
}
