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

    bool start() {
        http.route("/data/<arg>", QHttpServerRequest::Method::Post,
                   [this](const QString&, const QHttpServerRequest& request) {
                       saves.append(QJsonDocument::fromJson(request.body()).object());
                       return QHttpServerResponse(QJsonObject{{"ok", true}});
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
            api.invoice_details(QJsonDocument(QJsonObject{{"BusinessID", "STALE"}}));
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
};

QTEST_MAIN(BusinessSessionTests)
#include "BusinessSessionTests.moc"
