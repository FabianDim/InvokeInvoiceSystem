#include "Application/Controllers/AppController.h"
#include "View/UICode/Views/LoginPage.h"
#include "View/UICode/Views/LandingPage.h"
#include "Infrastructure/Enums/RouteEnums.h"
#include <QMessageBox>

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
    QObject::connect(main_->login_page(), &App::Views::LoginPage::navigate_to, this, &AppController::page_navigation);
    QObject::connect(main_->landing_page(), &App::Views::LandingPage::navigate_to, this, [this](Page page) {
        if (page == Page::Signup)
            page_navigation(Page::Signup);
    });
    QObject::connect(main_->signup_page(),
                     &App::Views::SignupPage::signup_requested,
                     api_,
                     &Infrastructure::Http::ApiClient::do_signup);
    QObject::connect(api_, &Infrastructure::Http::ApiClient::signup_succeeded, this, [this]() {
        page_navigation(Page::Login);
        main_->login_page()->set_status("Account created. You can now log in.");
    });
    QObject::connect(api_,
                     &Infrastructure::Http::ApiClient::signup_failed,
                     main_->signup_page(),
                     [this](const QString& message) { main_->signup_page()->set_status(message); });
    QObject::connect(api_, &Infrastructure::Http::ApiClient::login_succeeded, this, [this]() {
        main_->dashboard_page()->populate_business_list(QJsonDocument(QJsonObject{}));
        api_->get_business_list();
        page_navigation(Page::Dashboard);
    });
    QObject::connect(main_, &App::Views::MainWindow::logged_out, this, [this]() {
        api_->clear_session();
        main_->dashboard_page()->populate_business_list(QJsonDocument(QJsonObject{}));
        main_->new_invoice_stock_page()->reset_invoice();
    });
    QObject::connect(api_,
                     &Infrastructure::Http::ApiClient::login_failed,
                     main_->login_page(),
                     [this](const QString& message) { main_->login_page()->set_status(message); });
    QObject::connect(api_,
                     &Infrastructure::Http::ApiClient::business_list_received,
                     main_->dashboard_page(),
                     &App::Views::Dashboard::populate_business_list);
    QObject::connect(main_->dashboard_page(),
                     &App::Views::Dashboard::business_chosen,
                     api_,
                     &Infrastructure::Http::ApiClient::business_selected);
    QObject::connect(main_->dashboard_page(), &App::Views::Dashboard::business_chosen,
                     main_->items_page(), &App::Views::ItemsPage::business_selected);
    QObject::connect(main_->items_page(), &App::Views::ItemsPage::navigate_to,
                     this, &AppController::page_navigation);
    QObject::connect(main_->items_page(), &App::Views::ItemsPage::items_requested,
                     api_, &Infrastructure::Http::ApiClient::get_business_items);
    QObject::connect(api_, &Infrastructure::Http::ApiClient::business_items_received,
                     main_->items_page(), &App::Views::ItemsPage::populate_items);
    QObject::connect(api_, &Infrastructure::Http::ApiClient::business_items_failed,
                     main_->items_page(), &App::Views::ItemsPage::set_error);

    QObject::connect(main_->new_invoice_page(),
                     &App::Views::InvoiceDetailsInput::set_invoice_details,
                     api_,
                     &Infrastructure::Http::ApiClient::invoice_details);
    QObject::connect(api_, &Infrastructure::Http::ApiClient::invoice_started, this, [this]() {
        page_navigation(Page::StockInput);
    });
    QObject::connect(api_, &Infrastructure::Http::ApiClient::invoice_failed, this, [this](const QString& message) {
        QMessageBox::warning(main_, "Could not start invoice", message);
    });

    QObject::connect(
        main_->dashboard_page(), &App::Views::Dashboard::dash_navigation, this, &AppController::page_navigation);
    for (auto* form : {main_->client_page(),
                       main_->business_settings_page(),
                       main_->stock_settings_page(),
                       main_->account_settings_page()}) {
        QObject::connect(form, &App::Views::ManagementForm::navigate_to, this, &AppController::page_navigation);
        QObject::connect(
            form, &App::Views::ManagementForm::submit_resource, api_, &Infrastructure::Http::ApiClient::save_resource);
    }
    QObject::connect(api_, &Infrastructure::Http::ApiClient::resource_saved, this, &AppController::resource_saved);
    QObject::connect(api_,
                     &Infrastructure::Http::ApiClient::resource_save_failed,
                     this,
                     [this](const QString& message) {
                         main_->client_page()->set_status(message);
                         main_->stock_settings_page()->set_status(message);
                         main_->business_settings_page()->set_status(message);
                         main_->account_settings_page()->set_status(message);
                     });
    QObject::connect(main_->new_invoice_page(),
                     &App::Views::InvoiceDetailsInput::invoice_navigation,
                     this,
                     &AppController::page_navigation);
    QObject::connect(main_->new_invoice_stock_page(),
                     &App::Views::NewInvoiceStock::invoice_navigation,
                     this,
                     &AppController::page_navigation);

    QObject::connect(main_->new_invoice_stock_page(),
                     &App::Views::NewInvoiceStock::add_item_list_to_invoice,
                     api_,
                     &Infrastructure::Http::ApiClient::stock_list);
    QObject::connect(main_->new_invoice_stock_page(),
                     &App::Views::NewInvoiceStock::find_stock,
                     api_,
                     &Infrastructure::Http::ApiClient::get_stock_list);
    QObject::connect(api_,
                     &Infrastructure::Http::ApiClient::stock_list_received,
                     main_->new_invoice_stock_page(),
                     &App::Views::NewInvoiceStock::populate_stock_list);
}

void AppController::resource_saved(const QString& resource) {
    if (main_->items_page()->isVisible())
        main_->items_page()->load_items();
    if (resource == "client")
        main_->client_page()->set_status("Client saved.");
    else if (resource == "business") {
        main_->business_settings_page()->set_status("Business saved.");
        api_->get_business_list();
    } else if (resource == "stock")
        main_->stock_settings_page()->set_status("Stock item saved.");
    else
        main_->account_settings_page()->set_status("Account saved.");
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
    if ((page == Page::NewInvoice || page == Page::NewClient || page == Page::StockSettings ||
         page == Page::StockInput || page == Page::Items) && !main_->dashboard_page()->has_business()) {
        main_->show_page(main_->dashboard_page());
        return;
    }
    switch (page) {
    case Page::Landing:
        main_->show_page(main_->landing_page());
        break;
    case Page::Login:
        main_->show_page(main_->login_page());
        break;
    case Page::Signup:
        main_->show_page(main_->signup_page());
        break;
    case Page::Dashboard:
        main_->show_page(main_->dashboard_page());
        break;
    case Page::Items:
        main_->show_page(main_->items_page());
        main_->items_page()->load_items();
        break;
    case Page::NewInvoice:
        main_->new_invoice_stock_page()->reset_invoice();
        main_->show_page(main_->new_invoice_page());
        break;
    case Page::StockInput:
        api_->get_stock_list();
        main_->show_page(main_->new_invoice_stock_page());
        break;
    case Page::NewClient:
        main_->show_page(main_->client_page());
        break;
    case Page::BusinessSettings:
        main_->show_page(main_->business_settings_page());
        break;
    case Page::StockSettings:
        main_->show_page(main_->stock_settings_page());
        break;
    case Page::AccountSettings:
        main_->show_page(main_->account_settings_page());
        break;
    case Page::InvoiceBusinessChoice:
        main_->show_page(main_->dashboard_page());
        break;
    default:
        break;
    }
}
