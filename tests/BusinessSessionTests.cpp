#include <QtTest>
#include <QHttpServer>
#include <QTcpServer>
#include <QUrlQuery>
#include <QJsonArray>
#include <QFontDatabase>
#include <QMenuBar>
#include <QMenu>
#include "View/MainWindow.h"
#include "View/UICode/Views/Dashboard.h"
#include "View/UICode/Views/NewInvoiceStock.h"
#include "Infrastructure/Http/ApiClient.h"
#include "Application/Controllers/AppController.h"
#include "Utils/BusinessLogo.h"
#include <QTemporaryDir>

using App::Views::Dashboard;
using Infrastructure::Http::ApiClient;

namespace {
class TestAccountManager : public Invoke::Domain::Accounts::IAccountManager {
  public:
    bool logged_in = false;
    bool validEmail(const std::string&) override { return true; }
    bool validName(const std::string&) override { return true; }
    bool doesAccountExist(const std::string&) override { return true; }
    bool doesPasswordMatch(const std::string&) override { return true; }
    bool validatePassword(const std::string&) override { return true; }
    void createAccount(std::string&, std::string&) override {}
    void login(const std::string&, const std::string&) override { logged_in = true; }
    void logOut() override { logged_in = false; }
    bool is_logged_in() const override { return logged_in; }
    bool needsAccountSetup(const std::string&) override { return false; }
    std::shared_ptr<User> getAccount() override { return nullptr; }
};

QJsonDocument business_list() {
    return QJsonDocument(QJsonObject{
        {"BUSA", QJsonObject{{"BusinessID", "BUSA"}, {"BusinessName", "Alpha Plumbing"}}},
        {"BUSB", QJsonObject{{"BusinessID", "BUSB"}, {"BusinessName", "Beta Electrical"}}}});
}

QJsonDocument business_items(const QString& id) {
    return QJsonDocument(QJsonObject{
        {"BusinessID", id},
        {"businesses", QJsonArray{business_list().object().value(id)}},
        {"clients", QJsonArray{QJsonObject{{"ClientID", "CLI1"}, {"Name", "Example client"}, {"Email", "client@example.com"},
                                          {"Address", "10 Example Street, Sydney NSW 2000"}, {"Phone", "02 1234 5678"}}}},
        {"stocks", QJsonArray{QJsonObject{{"StockID", "STK1"}, {"Name", "Copper pipe"}, {"Quantity", 10},
                                         {"Price", 12.5}, {"Keywords", QJsonArray{"pipe", "copper"}}}}}});
}

QPushButton* button(Dashboard& dashboard, const QString& text) {
    for (auto* button : dashboard.findChildren<QPushButton*>()) {
        if (button->text() == text)
            return button;
    }
    return nullptr;
}

// Record real HTTP requests without connecting to MongoDB or writing business data.
struct RecordingServer {
    QHttpServer http;
    QTcpServer* tcp = new QTcpServer(&http);
    QList<QJsonObject> saves;
    QList<QJsonObject> invoices;
    QList<QString> stock_businesses;
    QList<QString> items_businesses;
    bool invalid_items = false;
    bool fail_items = false;
    bool fail_saves = false;

    bool start() {
        http.route("/business/items", QHttpServerRequest::Method::Get,
                   [this](const QHttpServerRequest& request) {
                       const auto id = QUrlQuery(request.url()).queryItemValue("BusinessID");
                       items_businesses.append(id);
                       if (fail_items)
                           return QHttpServerResponse(QJsonObject{{"error", "Database unavailable"}},
                                                      QHttpServerResponder::StatusCode::InternalServerError);
                       return QHttpServerResponse(invalid_items ? QJsonObject{} : business_items(id).object());
                   });
        http.route("/data/<arg>", QHttpServerRequest::Method::Post,
                   [this](const QString&, const QHttpServerRequest& request) {
                       saves.append(QJsonDocument::fromJson(request.body()).object());
                       if (fail_saves)
                           return QHttpServerResponse(QJsonObject{{"error", "Stock could not be saved"}},
                                                      QHttpServerResponder::StatusCode::InternalServerError);
                       return QHttpServerResponse(QJsonObject{{"ok", true}, {"id", "STKNEW"}});
                   });
        http.route("/invoices/invoice_start", QHttpServerRequest::Method::Post,
                   [this](const QHttpServerRequest& request) {
                       invoices.append(QJsonDocument::fromJson(request.body()).object());
                       return QHttpServerResponse(QJsonObject{{"ok", true}});
                   });
        http.route("/stock/list", QHttpServerRequest::Method::Get,
                   [this](const QHttpServerRequest& request) {
                       stock_businesses.append(QUrlQuery(request.url()).queryItemValue("BusinessID"));
                       return QHttpServerResponse(QJsonArray{});
                   });
        http.route("/auth/login", QHttpServerRequest::Method::Post, []() {
            return QHttpServerResponse(QJsonObject{{"ok", true}});
        });
        return tcp->listen(QHostAddress::LocalHost, 0) && http.bind(tcp);
    }

    QUrl url() const { return QUrl(QString("http://127.0.0.1:%1").arg(tcp->serverPort())); }
};
}

class BusinessSessionTests : public QObject {
    Q_OBJECT
  private slots:
    void initTestCase() {
#ifdef Q_OS_WIN
        QFontDatabase::addApplicationFont(qEnvironmentVariable("WINDIR") + "/Fonts/segoeui.ttf");
        QFontDatabase::addApplicationFont(qEnvironmentVariable("WINDIR") + "/Fonts/segoeuib.ttf");
#endif
        QFile stylesheet(":/styles/UI/Global.qss");
        QVERIFY(stylesheet.open(QIODevice::ReadOnly));
        const auto qss = QString::fromUtf8(stylesheet.readAll());
        QVERIFY(!qss.isEmpty());
        qApp->setStyleSheet(qss);
        QVERIFY(!QPixmap(":/icons/invoice_icon.png").isNull());
        QVERIFY(!QPixmap(":/icons/caret-down.xpm").isNull());
        QVERIFY(!QPixmap(":/icons/check.xpm").isNull());
    }

    void window_shows_only_current_page_and_updates_account_menu() {
        TestAccountManager account;
        App::Views::MainWindow window(account);
        window.resize(800, 800);
        window.show();
        QCoreApplication::processEvents();
        auto* stack = window.findChild<QStackedWidget*>();
        QVERIFY(stack);
        QVERIFY(stack->indexOf(window.landing_page()) >= 0);
        QCOMPARE(stack->currentWidget(), window.login_page());
        QVERIFY(window.landing_page()->isHidden());
        QVERIFY(window.login_page()->layout());
        QVERIFY(window.login_page()->width() >= 700);
        QVERIFY(window.grab().save("login-packaged-style.png"));

        account.logged_in = true;
        window.dashboard_page()->populate_business_list(business_list());
        window.show_page(window.dashboard_page());
        QCoreApplication::processEvents();
        QVERIFY(window.login_page()->isHidden());
        QVERIFY(window.landing_page()->isHidden());
        QCOMPARE(stack->currentWidget(), window.dashboard_page());
        auto* title = window.dashboard_page()->findChild<QLabel*>("titleLabel");
        QVERIFY(title);
        QCOMPARE(title->font().pixelSize(), 32);
        QVERIFY(window.grab().save("dashboard-packaged-style.png"));

        auto* menu = window.menuBar()->actions().at(1)->menu();
        QVERIFY(!menu->actions().at(0)->isVisible());
        QVERIFY(menu->actions().at(1)->isVisible());
        menu->actions().at(1)->trigger();
        QCoreApplication::processEvents();
        QVERIFY(!account.logged_in);
        QCOMPARE(stack->currentWidget(), window.landing_page());
        QVERIFY(window.dashboard_page()->isHidden());
        QVERIFY(menu->actions().at(0)->isVisible());
        QVERIFY(!menu->actions().at(1)->isVisible());
    }

    void dashboard_requires_selection_and_preserves_it_on_refresh() {
        Dashboard dashboard;
        auto* selector = dashboard.findChild<QComboBox*>("business_selector");
        QVERIFY(selector);
        QVERIFY(!dashboard.has_business());
        QVERIFY(!selector->isEnabled());
        QVERIFY(!button(dashboard, "Create stock item")->isEnabled());
        QVERIFY(button(dashboard, "Configure business")->isEnabled());
        QVERIFY(button(dashboard, "Account settings")->isEnabled());

        QSignalSpy chosen(&dashboard, &Dashboard::business_chosen);
        dashboard.populate_business_list(business_list());
        QVERIFY(selector->isEnabled());
        QVERIFY(!dashboard.has_business());
        selector->setCurrentIndex(2);
        QCOMPARE(chosen.last().at(0).toJsonObject().value("BusinessID").toString(), "BUSB");
        QVERIFY(button(dashboard, "Create stock item")->isEnabled());
        QVERIFY(button(dashboard, "Create new client")->isEnabled());
        QVERIFY(button(dashboard, "Create new invoice")->isEnabled());

        dashboard.populate_business_list(business_list());
        QCOMPARE(selector->currentData().toJsonObject().value("BusinessID").toString(), "BUSB");
        dashboard.resize(720, 420);
        dashboard.show();
        QCoreApplication::processEvents();
        QVERIFY(dashboard.grab().save("dashboard-business-selector.png"));

        dashboard.populate_business_list(QJsonDocument(QJsonObject{}));
        QVERIFY(!dashboard.has_business());
        QVERIFY(chosen.last().at(0).toJsonObject().isEmpty());
        QVERIFY(!button(dashboard, "Create new invoice")->isEnabled());
    }

    void controls_have_consistent_roles_and_geometry() {
        TestAccountManager account;
        App::Views::MainWindow window(account);
        window.resize(800, 800);
        window.show();
        const QList<QPair<QString, QWidget*>> pages{
            {"login", window.login_page()}, {"signup", window.signup_page()},
            {"dashboard", window.dashboard_page()}, {"invoice", window.new_invoice_page()},
            {"invoice-items", window.new_invoice_stock_page()}, {"client", window.client_page()},
            {"business", window.business_settings_page()}, {"stock", window.stock_settings_page()},
            {"account", window.account_settings_page()}, {"landing", window.landing_page()},
            {"records", window.items_page()},
            {"business-choice", window.business_invoice_choice_page()}};
        for (const auto& [name, page] : pages) {
            window.show_page(page);
            QTest::qWait(30);
            int button_height = -1;
            for (auto* button : page->findChildren<QPushButton*>()) {
                QVERIFY2(!button->property("role").toString().isEmpty(), qPrintable(name + ": " + button->text()));
                QVERIFY2(!button->property("controlSize").toString().isEmpty(), qPrintable(name + ": missing button size role"));
                if (!button->isVisible()) continue;
                if (button_height < 0) button_height = button->height();
                QCOMPARE(button->height(), button_height);
                const QRect bounds(button->mapTo(page, QPoint()), button->size());
                QVERIFY2(page->rect().contains(bounds), qPrintable(name + ": clipped " + button->text()));
            }
            QVERIFY(window.grab().save(name + "-consistent-ui.png"));
        }
        auto* dashboard = window.dashboard_page();
        QCOMPARE(button(*dashboard, "Create new invoice")->size(), button(*dashboard, "Create new client")->size());
        QCOMPARE(button(*dashboard, "Configure business")->size(), button(*dashboard, "Create stock item")->size());
        window.show_page(window.new_invoice_page());
        QCoreApplication::processEvents();
        auto* invoice = window.new_invoice_page();
        const auto height = invoice->findChild<QLineEdit*>("invoice_number")->height();
        QCOMPARE(invoice->findChild<QComboBox*>("invoice_theme")->height(), height);
        QCOMPARE(invoice->findChild<QDateEdit*>("date_created")->height(), height);
        QCOMPARE(invoice->findChild<QPushButton*>("browse_folder_button")->height(), height);
    }

    void resource_saves_follow_dashboard_selection() {
        RecordingServer server;
        QVERIFY(server.start());
        ApiClient api(server.url(), nullptr);
        Dashboard dashboard;
        connect(&dashboard, &Dashboard::business_chosen, &api, &ApiClient::business_selected);
        dashboard.populate_business_list(business_list());
        auto* selector = dashboard.findChild<QComboBox*>("business_selector");
        QSignalSpy failed(&api, &ApiClient::resource_save_failed);
        QSignalSpy saved(&api, &ApiClient::resource_saved);
        const QJsonDocument payload(QJsonObject{{"Name", "Example"}, {"BusinessID", "STALE"}});
        api.save_resource("stock", payload);
        QCOMPARE(failed.count(), 1);
        QVERIFY(server.saves.isEmpty());

        selector->setCurrentIndex(1);
        api.save_resource("stock", payload);
        QTRY_COMPARE(saved.count(), 1);
        QCOMPARE(server.saves.last().value("BusinessID").toString(), "BUSA");
        selector->setCurrentIndex(2);
        api.save_resource("client", payload);
        QTRY_COMPARE(saved.count(), 2);
        QCOMPARE(server.saves.last().value("BusinessID").toString(), "BUSB");
        QCOMPARE(payload.object().value("BusinessID").toString(), "STALE");

        dashboard.populate_business_list(QJsonDocument(QJsonObject{}));
        api.save_resource("client", payload);
        QCOMPARE(failed.count(), 2);
        QCOMPARE(server.saves.count(), 2);
        api.save_resource("business", QJsonDocument(QJsonObject{{"Business name", "New business"}}));
        QTRY_COMPARE(saved.count(), 3);
        QVERIFY(!server.saves.last().contains("BusinessID"));
    }

    void invoices_and_stock_requests_use_session_business() {
        RecordingServer server;
        QVERIFY(server.start());
        ApiClient api(server.url(), nullptr);
        QSignalSpy failed(&api, &ApiClient::invoice_failed);
        QSignalSpy started(&api, &ApiClient::invoice_started);
        QSignalSpy stock(&api, &ApiClient::stock_list_received);
        api.invoice_details(QJsonDocument(QJsonObject{}));
        QCOMPARE(failed.count(), 1);
        QVERIFY(server.invoices.isEmpty());
        for (const auto& id : {QString("BUSA"), QString("BUSB")}) {
            api.business_selected(QJsonObject{{"BusinessID", id}});
            api.invoice_details(QJsonDocument(QJsonObject{{"BusinessID", "STALE"}, {"ClientID", "CLI1"}}));
            QTRY_VERIFY(!server.invoices.isEmpty() && server.invoices.last().value("BusinessID").toString() == id);
            api.get_stock_list();
            QTRY_VERIFY(!server.stock_businesses.isEmpty() && server.stock_businesses.last() == id);
        }
        QTRY_COMPARE(started.count(), 2);
        QTRY_COMPARE(stock.count(), 2);
    }

    void login_clears_previous_business() {
        RecordingServer server;
        QVERIFY(server.start());
        ApiClient api(server.url(), nullptr);
        QSignalSpy logged_in(&api, &ApiClient::login_succeeded);
        QSignalSpy failed(&api, &ApiClient::resource_save_failed);
        api.business_selected(QJsonObject{{"BusinessID", "BUSA"}});
        api.do_login("test@example.com", "test", false);
        QTRY_COMPARE(logged_in.count(), 1);
        api.save_resource("stock", QJsonDocument(QJsonObject{}));
        QCOMPARE(failed.count(), 1);
        QVERIFY(server.saves.isEmpty());
    }

    void new_invoice_discards_previous_stock() {
        App::Views::NewInvoiceStock page;
        const QJsonObject item{{"Name", "Old business stock"}, {"Quantity", 2}, {"Price", 10.0}};
        page.stock_items.append(item);
        emit page.add_item_to_invoice(item);
        page.populate_stock_list(QJsonDocument(QJsonArray{item}));
        page.reset_invoice();
        QVERIFY(page.stock_items.isEmpty());
        QCOMPARE(qobject_cast<QComboBox*>(page.invoice_body_form_fields.value("stock_selector"))->count(), 0);
        for (auto* label : page.findChildren<QLabel*>())
            QVERIFY(label->text() != "Old business stock");
    }

    void invoice_clients_populate_and_submit_for_active_business() {
        RecordingServer server;
        QVERIFY(server.start());
        TestAccountManager account;
        ApiClient api(server.url(), &account);
        App::Views::MainWindow window(account);
        Application::Controllers::AppController controller(&window, account, &api);
        window.resize(800, 850);
        window.show();
        auto* dashboard = window.dashboard_page();
        dashboard->populate_business_list(business_list());
        auto* businesses = dashboard->findChild<QComboBox*>("business_selector");
        businesses->setCurrentIndex(1);
        controller.page_navigation(Page::Items);
        QTRY_COMPARE(window.items_page()->findChild<QTableWidget*>("client_records")->rowCount(), 1);
        controller.page_navigation(Page::NewInvoice);
        auto* page = window.new_invoice_page();
        auto* clients = page->findChild<QComboBox*>("invoice_client");
        auto* next = page->findChild<QPushButton*>("invoice_next");
        QCOMPARE(clients->count(), 2);
        QCOMPARE(server.items_businesses.size(), 1); // Reuses the business-records cache.
        QVERIFY(!next->isEnabled());
        clients->setCurrentIndex(1);
        QVERIFY(next->isEnabled());
        const auto preview = page->findChild<QLabel*>("invoice_client_details")->text();
        QVERIFY(preview.contains("client@example.com"));
        QVERIFY(preview.contains("10 Example Street"));
        QCoreApplication::processEvents();
        QVERIFY(window.grab().save("invoice-client-selection.png"));
        next->click();
        QTRY_COMPARE(server.invoices.size(), 1);
        QCOMPARE(server.invoices.first().value("ClientID").toString(), "CLI1");
        QCOMPARE(server.invoices.first().value("BusinessID").toString(), "BUSA");
        QVERIFY(!server.invoices.first().contains("website"));
        QVERIFY(!page->findChild<QLineEdit*>("website"));
        QTRY_COMPARE(window.findChild<QStackedWidget*>()->currentWidget(), window.new_invoice_stock_page());
        controller.page_navigation(Page::Dashboard);
        businesses->setCurrentIndex(2);
        QCOMPARE(clients->count(), 1);
        QVERIFY(!next->isEnabled());
        QVERIFY(page->findChild<QLabel*>("invoice_client_details")->text().isEmpty());
        page->populate_clients(business_items("BUSA"));
        QCOMPARE(clients->count(), 1);
        controller.page_navigation(Page::NewInvoice);
        QTRY_COMPARE(clients->count(), 2);
        QCOMPARE(server.items_businesses.last(), "BUSB");
        clients->setCurrentIndex(1);
        emit window.logged_out();
        QVERIFY(!next->isEnabled());
        QCOMPARE(clients->count(), 1);
    }

    void invoice_clients_empty_error_and_refresh_states() {
        App::Views::InvoiceDetailsInput page;
        page.business_selected(QJsonObject{{"BusinessID", "BUSA"}});
        auto* clients = page.findChild<QComboBox*>("invoice_client");
        auto* next = page.findChild<QPushButton*>("invoice_next");
        auto* refresh = page.findChild<QPushButton*>("refresh_invoice_clients");
        auto empty = business_items("BUSA").object();
        empty["clients"] = QJsonArray{};
        page.populate_clients(QJsonDocument(empty));
        QVERIFY(!clients->isEnabled());
        QVERIFY(!next->isEnabled());
        QVERIFY(page.findChild<QLabel*>("invoice_client_status")->text().contains("No clients saved"));
        page.populate_clients(business_items("BUSA"));
        clients->setCurrentIndex(1);
        QVERIFY(next->isEnabled());
        QSignalSpy requests(&page, &App::Views::InvoiceDetailsInput::clients_requested);
        refresh->click();
        QCOMPARE(requests.count(), 1);
        QCOMPARE(requests.first().first().toBool(), true);
        QVERIFY(!next->isEnabled());
        page.set_client_error("Database unavailable");
        QVERIFY(refresh->isEnabled());
        QVERIFY(!next->isEnabled());
        QVERIFY(page.findChild<QLabel*>("invoice_client_details")->text().isEmpty());
    }

    void invoice_requires_a_client_before_posting() {
        RecordingServer server;
        QVERIFY(server.start());
        ApiClient api(server.url(), nullptr);
        api.business_selected(QJsonObject{{"BusinessID", "BUSA"}});
        QSignalSpy failed(&api, &ApiClient::invoice_failed);
        api.invoice_details(QJsonDocument(QJsonObject{}));
        QCOMPARE(failed.count(), 1);
        QVERIFY(server.invoices.isEmpty());
    }

    void new_invoice_stock_can_be_temporary_or_saved() {
        RecordingServer server;
        QVERIFY(server.start());
        ApiClient api(server.url(), nullptr);
        api.business_selected(QJsonObject{{"BusinessID", "BUSA"}});
        App::Views::NewInvoiceStock page;
        page.business_selected(QJsonObject{{"BusinessID", "BUSA"}});
        connect(&page, &App::Views::NewInvoiceStock::save_invoice_stock, &api, &ApiClient::save_invoice_stock);
        connect(&api, &ApiClient::invoice_stock_saved, &page, &App::Views::NewInvoiceStock::stock_saved);
        connect(&api, &ApiClient::invoice_stock_save_failed, &page, &App::Views::NewInvoiceStock::stock_save_failed);
        auto* name = page.findChild<QLineEdit*>("invoice_new_stock_name");
        auto* quantity = qobject_cast<QLineEdit*>(page.invoice_body_form_fields.value("quantity"));
        auto* price = qobject_cast<QLineEdit*>(page.invoice_body_form_fields.value("price"));
        auto* add = page.findChild<QPushButton*>("invoice_add_item");
        auto* save = page.findChild<QCheckBox*>("save_invoice_stock");
        name->setText("Installation labour");
        quantity->setText("0");
        price->setText("75.50");
        add->click();
        QVERIFY(page.stock_items.isEmpty());
        quantity->setText("2");
        add->click();
        QCOMPARE(page.stock_items.size(), 1);
        QVERIFY(server.saves.isEmpty());
        QCOMPARE(page.stock_items.first().toObject().value("Name").toString(), "Installation labour");
        QCOMPARE(page.stock_items.first().toObject().value("Price").toDouble(), 75.5);

        name->setText("Copper fitting");
        quantity->setText("3");
        price->setText("12.50");
        save->setChecked(true);
        page.findChild<QSpinBox*>("new_stock_on_hand")->setValue(100);
        page.findChild<QLineEdit*>("new_stock_unit")->setText("each");
        page.resize(800, 950);
        page.show();
        QCoreApplication::processEvents();
        QVERIFY(page.grab().save("invoice-new-stock.png"));
        add->click();
        QVERIFY(!add->isEnabled());
        QCOMPARE(page.stock_items.size(), 1);
        QTRY_COMPARE(page.stock_items.size(), 2);
        QCOMPARE(server.saves.size(), 1);
        QCOMPARE(server.saves.first().value("Quantity").toInt(), 100);
        QCOMPARE(server.saves.first().value("BusinessID").toString(), "BUSA");
        QCOMPARE(page.stock_items.last().toObject().value("Quantity").toInt(), 3);
        QCOMPARE(page.stock_items.last().toObject().value("StockID").toString(), "STKNEW");

        page.findChild<QComboBox*>("invoice_item_source")->setCurrentIndex(1);
        quantity->setText("1");
        add->click(); // Uses the saved price when no override is entered.
        QCOMPARE(page.stock_items.size(), 3);
        QCOMPARE(page.stock_items.last().toObject().value("Price").toDouble(), 12.5);
        QCOMPARE(server.saves.size(), 1);
    }

    void invoice_stock_save_failure_and_late_reply_preserve_invoice() {
        App::Views::NewInvoiceStock page;
        auto* name = page.findChild<QLineEdit*>("invoice_new_stock_name");
        auto* quantity = qobject_cast<QLineEdit*>(page.invoice_body_form_fields.value("quantity"));
        auto* price = qobject_cast<QLineEdit*>(page.invoice_body_form_fields.value("price"));
        auto* save = page.findChild<QCheckBox*>("save_invoice_stock");
        auto* add = page.findChild<QPushButton*>("invoice_add_item");
        name->setText("New item");
        quantity->setText("2");
        price->setText("10");
        save->setChecked(true);
        page.findChild<QLineEdit*>("new_stock_unit")->setText("each");
        QSignalSpy requests(&page, &App::Views::NewInvoiceStock::save_invoice_stock);
        add->click();
        QCOMPARE(requests.count(), 1);
        const auto request_id = requests.first().at(1).toULongLong();
        page.stock_save_failed("Database unavailable", request_id);
        QCOMPARE(name->text(), "New item");
        QCOMPARE(quantity->text(), "2");
        QVERIFY(page.stock_items.isEmpty());
        QVERIFY(add->isEnabled());
        save->setChecked(false);
        add->click();
        QCOMPARE(page.stock_items.size(), 1);
        page.reset_invoice();
        page.stock_saved(qvariant_cast<QJsonDocument>(requests.first().at(0)), request_id);
        QVERIFY(page.stock_items.isEmpty());
    }

    void invoice_stock_api_reports_failures_and_invalidates_cache() {
        RecordingServer server;
        QVERIFY(server.start());
        ApiClient api(server.url(), nullptr);
        api.business_selected(QJsonObject{{"BusinessID", "BUSA"}});
        QSignalSpy items(&api, &ApiClient::business_items_received);
        api.get_business_items();
        QTRY_COMPARE(items.count(), 1);
        QSignalSpy saved(&api, &ApiClient::invoice_stock_saved);
        QSignalSpy failed(&api, &ApiClient::invoice_stock_save_failed);
        const QJsonDocument stock(QJsonObject{{"Name", "New"}, {"Quantity", 2}, {"StockOnHand", 10}, {"Price", 3}});
        server.fail_saves = true;
        api.save_invoice_stock(stock, 1);
        QTRY_COMPARE(failed.count(), 1);
        QCOMPARE(failed.first().at(1).toULongLong(), 1);
        QVERIFY(saved.isEmpty());
        server.fail_saves = false;
        api.save_invoice_stock(stock, 2);
        QTRY_COMPARE(saved.count(), 1);
        api.get_business_items();
        QTRY_COMPARE(items.count(), 2);
        QCOMPARE(server.items_businesses.size(), 2);
    }

    void business_form_includes_website_and_portable_logo() {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const auto path = directory.filePath("logo.png");
        QImage image(50, 50, QImage::Format_ARGB32);
        image.fill(QColor("#2a9d8f"));
        QVERIFY(image.save(path));
        TestAccountManager account;
        App::Views::MainWindow window(account);
        auto* form = window.business_settings_page();
        form->select_logo(path);
        QCOMPARE(form->findChild<QLineEdit*>("business_logo_path")->text(), path);
        for (auto* input : form->findChildren<QLineEdit*>("form_input"))
            input->setText(input->placeholderText() == "Website" ? "https://example.com" : "Example");
        QSignalSpy submitted(form, &App::Views::ManagementForm::submit_resource);
        form->findChild<QPushButton*>("register_button_")->click();
        QCOMPARE(submitted.count(), 1);
        const auto data = qvariant_cast<QJsonDocument>(submitted.first().at(1)).object();
        QCOMPARE(data.value("Website").toString(), "https://example.com");
        QCOMPARE(data.value("LogoPath").toString(), path);
        const auto bytes = QByteArray::fromBase64(data.value("LogoData").toString().toLatin1());
        QVERIFY(!QImage::fromData(bytes, "PNG").isNull());
        QVERIFY(form->findChild<QLineEdit*>("business_logo_path")->text().isEmpty());
        const auto oversized = directory.filePath("large.png");
        QFile file(oversized);
        QVERIFY(file.open(QIODevice::WriteOnly));
        QVERIFY(file.resize(BusinessLogo::maximum_bytes + 1));
        file.close();
        form->select_logo(oversized);
        QVERIFY(form->findChild<QLineEdit*>("business_logo_path")->text().isEmpty());
        QByteArray normalised;
        QString error;
        QVERIFY(!BusinessLogo::normalise("not an image", normalised, error));
        QImage large(1500, 500, QImage::Format_RGB32);
        large.fill(Qt::blue);
        QByteArray source;
        QBuffer buffer(&source);
        buffer.open(QIODevice::WriteOnly);
        QVERIFY(large.save(&buffer, "JPEG"));
        QVERIFY(BusinessLogo::normalise(source, normalised, error));
        QCOMPARE(QImage::fromData(normalised).width(), 1024);
    }

    void business_records_navigation_and_tables() {
        RecordingServer server;
        QVERIFY(server.start());
        TestAccountManager account;
        ApiClient api(server.url(), &account);
        App::Views::MainWindow window(account);
        Application::Controllers::AppController controller(&window, account, &api);
        window.resize(1000, 650);
        window.show();
        auto* dashboard = window.dashboard_page();
        auto* browse = button(*dashboard, "Browse business records");
        QVERIFY(browse);
        QVERIFY(!browse->isEnabled());
        controller.page_navigation(Page::Items);
        QCOMPARE(window.findChild<QStackedWidget*>()->currentWidget(), dashboard);
        dashboard->populate_business_list(business_list());
        dashboard->findChild<QComboBox*>("business_selector")->setCurrentIndex(1);
        QVERIFY(browse->isEnabled());
        browse->click();
        auto* page = window.items_page();
        QCOMPARE(window.findChild<QStackedWidget*>()->currentWidget(), page);
        auto* stock = page->findChild<QTableWidget*>("stock_records");
        QTRY_COMPARE(stock->rowCount(), 1);
        QCOMPARE(stock->item(0, 0)->text(), "Copper pipe");
        QCOMPARE(stock->item(0, 2)->data(Qt::DisplayRole).toInt(), 10);
        QCOMPARE(stock->item(0, 6)->text(), "pipe, copper");
        QCOMPARE(stock->editTriggers(), QAbstractItemView::NoEditTriggers);
        auto* tabs = page->findChild<QTabWidget*>();
        QCOMPARE(tabs->count(), 3);
        tabs->setCurrentIndex(1);
        tabs->setCurrentIndex(2);
        QCOMPARE(server.items_businesses.size(), 1);
        QVERIFY(window.grab().save("business-records-stock.png"));
        controller.page_navigation(Page::Dashboard);
        browse->click();
        QCOMPARE(server.items_businesses.size(), 1);
        page->findChild<QPushButton*>("refresh_items")->click();
        QTRY_COMPARE(server.items_businesses.size(), 2);
        QTRY_VERIFY(page->findChild<QPushButton*>("refresh_items")->isEnabled());
        auto empty = business_items("BUSA").object();
        empty["clients"] = QJsonArray{};
        empty["stocks"] = QJsonArray{};
        page->populate_items(QJsonDocument(empty));
        QCOMPARE(stock->rowCount(), 0);
        QCOMPARE(tabs->tabText(2), "Stock (0)");
        page->populate_items(business_items("BUSB"));
        QCOMPARE(stock->rowCount(), 0);
        emit window.logged_out();
        QVERIFY(!browse->isEnabled());
        QCOMPARE(page->findChild<QTableWidget*>("business_records")->rowCount(), 0);
    }

    void business_records_cache_refresh_save_and_selection() {
        RecordingServer server;
        QVERIFY(server.start());
        ApiClient api(server.url(), nullptr);
        QSignalSpy received(&api, &ApiClient::business_items_received);
        QSignalSpy saved(&api, &ApiClient::resource_saved);
        api.business_selected(QJsonObject{{"BusinessID", "BUSA"}});
        api.get_business_items();
        api.get_business_items();
        QTRY_COMPARE(received.count(), 1);
        QCOMPARE(server.items_businesses.size(), 1);
        api.get_business_items();
        QCOMPARE(received.count(), 2);
        QCOMPARE(server.items_businesses.size(), 1);
        api.get_business_items(true);
        QTRY_COMPARE(received.count(), 3);
        QCOMPARE(server.items_businesses.size(), 2);
        for (const auto& resource : {QString("client"), QString("stock"), QString("business")}) {
            api.save_resource(resource, QJsonDocument(QJsonObject{}));
            QTRY_COMPARE(saved.count(), 1);
            saved.clear();
            const auto previous = received.count();
            api.get_business_items();
            QTRY_COMPARE(received.count(), previous + 1);
        }
        QCOMPARE(server.items_businesses.size(), 5);
        api.business_selected(QJsonObject{{"BusinessID", "BUSB"}});
        api.get_business_items();
        QTRY_COMPARE(received.count(), 7);
        QCOMPARE(server.items_businesses.last(), "BUSB");
        QCOMPARE(qvariant_cast<QJsonDocument>(received.last().at(0)).object().value("BusinessID").toString(), "BUSB");
        api.clear_session();
        api.business_selected(QJsonObject{{"BusinessID", "BUSB"}});
        api.get_business_items();
        QTRY_COMPARE(received.count(), 8);
        QCOMPARE(server.items_businesses.size(), 7);
    }

    void business_records_failures_are_retryable_and_not_cached() {
        RecordingServer server;
        QVERIFY(server.start());
        ApiClient api(server.url(), nullptr);
        QSignalSpy received(&api, &ApiClient::business_items_received);
        QSignalSpy failed(&api, &ApiClient::business_items_failed);
        api.get_business_items();
        QCOMPARE(failed.count(), 1);
        QVERIFY(server.items_businesses.isEmpty());
        api.business_selected(QJsonObject{{"BusinessID", "BUSA"}});
        server.invalid_items = true;
        api.get_business_items();
        QTRY_COMPARE(failed.count(), 2);
        server.invalid_items = false;
        server.fail_items = true;
        api.get_business_items();
        QTRY_COMPARE(failed.count(), 3);
        QCOMPARE(failed.last().at(0).toString(), "Database unavailable");
        server.fail_items = false;
        api.get_business_items();
        QTRY_COMPARE(received.count(), 1);
        QCOMPARE(server.items_businesses.size(), 3);
    }

    void business_records_pending_requests_cannot_cross_sessions() {
        RecordingServer server;
        QVERIFY(server.start());
        ApiClient api(server.url(), nullptr);
        QSignalSpy received(&api, &ApiClient::business_items_received);
        QSignalSpy failed(&api, &ApiClient::business_items_failed);
        api.business_selected(QJsonObject{{"BusinessID", "BUSA"}});
        api.get_business_items();
        api.business_selected(QJsonObject{{"BusinessID", "BUSB"}});
        api.get_business_items();
        api.clear_session();
        api.business_selected(QJsonObject{{"BusinessID", "BUSA"}});
        api.get_business_items();
        QTRY_COMPARE(received.count(), 1);
        QCOMPARE(qvariant_cast<QJsonDocument>(received.at(0).at(0)).object().value("BusinessID").toString(), "BUSA");
        QVERIFY(failed.isEmpty());
    }
};

QTEST_MAIN(BusinessSessionTests)
#include "BusinessSessionTests.moc"
