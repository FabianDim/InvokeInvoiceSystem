#include "Application/Controllers/AppController.h"
#include "View/UICode/Views/LoginPage.h"
#include "View/UICode/Views/LandingPage.h"
#include "Infrastructure/Enums/RouteEnums.h"

using namespace Application::Controllers;
/*define connections from the front end to the back end and the page navigations.*/
AppController::AppController(App::Views::MainWindow* main,
                             Invoke::Domain::Accounts::IAccountManager& accountManager,
                             Infrastructure::Http::ApiClient* api,
                             QObject* parent)
    : QObject(parent), main_(main), accountManager_(accountManager), api_(api) {
    QObject::connect(
        main_->landing_page(), &App::Views::LandingPage::navigate_to, this, &AppController::page_navigation);
    QObject::connect(
        main_->login_page(), &App::Views::LoginPage::login_requested, api_, &Infrastructure::Http::ApiClient::do_login);
    QObject::connect(main->business_invoice_choice_page(),
                     &App::Views::BusinessInvoiceChoice::find_businesses,
                     api_,
                     &Infrastructure::Http::ApiClient::get_business_list);

    QObject::connect(api_,
                     &Infrastructure::Http::ApiClient::business_list_received,
                     main_->business_invoice_choice_page(),
                     &App::Views::BusinessInvoiceChoice::populate_business_list);

    QObject::connect(main_->business_invoice_choice_page(),
                     &App::Views::BusinessInvoiceChoice::navigate_to,
                     this,
                     &AppController::page_navigation);
    QObject::connect(main_->business_invoice_choice_page(),
                     &App::Views::BusinessInvoiceChoice::business_chosen,
                     api_,
                     &Infrastructure::Http::ApiClient::business_selected);

    QObject::connect(main_->new_invoice_page(),
                     &App::Views::InvoiceDetailsInput::set_invoice_details,
                     api_,
                     &Infrastructure::Http::ApiClient::invoice_details);

    QObject::connect(
        main_->dashboard_page(), &App::Views::Dashboard::dash_navigation, this, &AppController::page_navigation);
    QObject::connect(main_->new_invoice_page(),
                     &App::Views::InvoiceDetailsInput::invoice_navigation,
                     this,
                     &AppController::page_navigation);

    QObject::connect(main_->new_invoice_stock_page(),
                     &App::Views::NewInvoiceStock::add_item_list_to_invoice,
                     api_,
                     &Infrastructure::Http::ApiClient::stock_list);

    emit main_->business_invoice_choice_page()->find_businesses();
}

/**
 * @brief Navigate between pages using the enum
 *
 * Uses a switch statement to navigate between pages using the
 * page enum. Frontend pages can call the enum from anywhere.
 *
 * @param page An enum containing the name of all the pages
 * @return Void
 * @pre All the pages should be built in the main page
 */
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
    case Page::StockInput:
        main_->show_page(main_->new_invoice_stock_page());
        break;
    case Page::InvoiceBusinessChoice:
        main_->show_page(main_->business_invoice_choice_page());
        break;
    default:
        break;
    }
}
