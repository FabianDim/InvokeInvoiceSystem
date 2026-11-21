#include "Domain/Records/RecordDefinition.h"
#include "Domain/Invoices/InvoiceFilename.h"
#include "Domain/Invoices/InvoiceRecord.h"
#include "Application/Accounts/FrontendAccountSession.h"
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
#include "Infrastructure/Http/AuthSessions.h"
#include "Application/Controllers/AppController.h"
#include "Utils/BusinessLogo.h"
#include <QTemporaryDir>
#include <QSettings>
#include <QClipboard>
#include <QScrollArea>
#include <QScrollBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include "Domain/Stock/StockShortage.h"
#include "View/UICode/Views/InvoicesPage.h"
#include "Infrastructure/Pdf/InvoicePdfGenerator.h"
#include <QTableWidget>
#include <QCheckBox>

using App::Views::Dashboard;
using Infrastructure::Http::ApiClient;

namespace {
class TestAccountManager : public Invoke::Domain::Accounts::IAccountManager {
  public:
    bool logged_in = false;
    std::shared_ptr<User> user;
    bool validEmail(const std::string&) override { return true; }
    bool validName(const std::string&) override { return true; }
    bool doesAccountExist(const std::string&) override { return true; }
    bool doesPasswordMatch(const std::string&) override { return true; }
    bool validatePassword(const std::string&) override { return true; }
    void createAccount(std::string&, std::string&) override {}
    void login(const std::string&, const std::string&) override { logged_in = true; }
    void logOut() override { logged_in = false; user.reset(); }
    bool is_logged_in() const override { return logged_in; }
    bool needsAccountSetup(const std::string&) override { return false; }
    std::shared_ptr<User> getAccount() override { return user; }
    void set_authenticated_user(std::shared_ptr<User> identity) override { user = std::move(identity); logged_in = true; }
};

QJsonDocument business_list() {
    return QJsonDocument(QJsonObject{
        {"65a000000000000000000001", QJsonObject{{"_id", "65a000000000000000000001"}, {"BusinessName", "Alpha Plumbing"}}},
        {"65a000000000000000000002", QJsonObject{{"_id", "65a000000000000000000002"}, {"BusinessName", "Beta Electrical"}}}});
}

QJsonDocument business_items(const QString& id) {
    return QJsonDocument(QJsonObject{
        {"BusinessID", id},
        {"businesses", QJsonArray{business_list().object().value(id)}},
        {"clients", QJsonArray{QJsonObject{{"_id", "65b000000000000000000001"}, {"Name", "Example client"}, {"Email", "client@example.com"},
                                          {"Address", "10 Example Street, Sydney NSW 2000"}, {"Phone", "02 1234 5678"}}}},
        {"stocks", QJsonArray{QJsonObject{{"_id", "65c000000000000000000001"}, {"Name", "Copper pipe"}, {"Quantity", 10},
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
    QByteArray pdf_body = "%PDF-1.4\ntransport test fixture\n%%EOF\n";
    QByteArray pdf_type = "application/pdf";
    int pdf_status = 200;
    int pdf_requests = 0;
    QString assigned_number;
    QList<QJsonDocument> pdf_payloads;

    bool start() {
        http.route("/invoices/stock-list", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest& request) {
            ++pdf_requests;
            pdf_payloads.append(QJsonDocument::fromJson(request.body()));
            return QHttpServerResponse(pdf_type, pdf_body, static_cast<QHttpServerResponder::StatusCode>(pdf_status));
        });
        http.route("/business/list", QHttpServerRequest::Method::Get, []() { return QHttpServerResponse(business_list().object()); });
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
                   [this](const QString& resource, const QHttpServerRequest& request) {
                       saves.append(QJsonDocument::fromJson(request.body()).object());
                       if (fail_saves)
                           return QHttpServerResponse(QJsonObject{{"error", "Stock could not be saved"}},
                                                      QHttpServerResponder::StatusCode::InternalServerError);
                       return QHttpServerResponse(QJsonObject{{"ok", true}, {"_id", resource == "business" ? "65a000000000000000000001" : "65c000000000000000000002"}});
                   });
        http.route("/invoices/invoice_start", QHttpServerRequest::Method::Post,
                   [this](const QHttpServerRequest& request) {
                       invoices.append(QJsonDocument::fromJson(request.body()).object());
                       return QHttpServerResponse(QJsonObject{{"ok", true}, {"invoice_number", assigned_number}});
                   });
        http.route("/stock/list", QHttpServerRequest::Method::Get,
                   [this](const QHttpServerRequest& request) {
                       stock_businesses.append(QUrlQuery(request.url()).queryItemValue("BusinessID"));
                       return QHttpServerResponse(QJsonArray{});
                   });
        http.route("/auth/login", QHttpServerRequest::Method::Post, []() {
            return QHttpServerResponse(QJsonObject{
                {"ok", true}, {"token", "recording-session"},
                {"user", QJsonObject{{"_id", "65d000000000000000000003"}, {"email", "test@example.invalid"}}}});
        });
        return tcp->listen(QHostAddress::LocalHost, 0) && http.bind(tcp);
    }

    QUrl url() const { return QUrl(QString("http://127.0.0.1:%1").arg(tcp->serverPort())); }
};
}

class BusinessSessionTests : public QObject {
    Q_OBJECT
    QTemporaryDir settings_directory_;
  private slots:

    void record_popup_edits_business_and_client_data() {
        QTest::addColumn<QString>("resource");
        QTest::addColumn<int>("tab");
        QTest::newRow("business") << QString("business") << 0;
        QTest::newRow("client") << QString("client") << 1;
    }

    void record_popup_edits_business_and_client() {
        QFETCH(QString, resource);
        QFETCH(int, tab);
        ApiClient api(QUrl("http://127.0.0.1:1"), nullptr);
        api.set_offline(true);
        QSignalSpy businesses(&api, &ApiClient::business_list_received);
        api.save_resource("business", QJsonDocument(QJsonObject{{"Business name", "Original business"},
            {"Address", "Original address"}, {"InvoicePrefix", "KEEP-"}, {"InvoiceTerms", "Keep terms"},
            {"GSTRegistered", true}, {"PaymentTerms", "14"}, {"BankName", "Original bank"}}));
        api.get_business_list();
        const auto list = qvariant_cast<QJsonDocument>(businesses.last().first()).object();
        const auto business = list.begin().value().toObject();
        api.business_selected(business);
        api.save_resource("client", QJsonDocument(QJsonObject{{"Name", "Original client"}, {"Address", "Client address"}}));
        App::Views::ItemsPage page;
        page.business_selected(business);
        connect(&page, &App::Views::ItemsPage::items_requested, &api, &ApiClient::get_business_items);
        connect(&api, &ApiClient::business_items_received, &page, &App::Views::ItemsPage::populate_items);
        connect(&page, &App::Views::ItemsPage::record_update_requested, &api, &ApiClient::update_record);
        connect(&api, &ApiClient::record_updated, &page, &App::Views::ItemsPage::record_updated);
        connect(&api, &ApiClient::record_update_failed, &page, &App::Views::ItemsPage::record_update_failed);
        QSignalSpy updates(&api, &ApiClient::record_updated);
        QSignalSpy requests(&page, &App::Views::ItemsPage::record_update_requested);
        page.load_items();
        page.resize(950, 650); page.show();
        auto* tabs = page.findChild<QTabWidget*>("business_items_tabs");
        tabs->setCurrentIndex(tab);
        auto* table = qobject_cast<QTableWidget*>(tabs->currentWidget());
        auto* edit = page.findChild<QPushButton*>("edit_record");
        QCOMPARE(edit->text(), "Edit " + resource);
        QVERIFY(!edit->isEnabled());
        table->selectRow(0);
        QVERIFY(edit->isEnabled());
        const auto id = table->item(0, 0)->data(Qt::UserRole);
        edit->click();
        const auto prefix = resource + "_edit_";
        QPointer<QDialog> popup = page.findChild<QDialog*>(prefix + "dialog");
        QVERIFY(popup);
        QVERIFY(popup->findChild<App::Views::ManagementForm*>());
        QCOMPARE(popup->windowTitle(), "Edit " + resource);
        QVERIFY(!popup->findChild<QLineEdit*>(prefix + "quantity"));
        const auto name = prefix + (resource == "business" ? "businessname" : "name");
        const auto address = prefix + (resource == "business" ? "businessaddress" : "address");
        QCOMPARE(popup->findChild<QLineEdit*>(address)->text(), resource == "business" ? "Original address" : "Client address");
        popup->findChild<QLineEdit*>(name)->clear();
        popup->findChild<QPushButton*>(prefix + "save")->click();
        QCOMPARE(requests.size(), 0);
        QVERIFY(!popup->findChild<QLabel*>("record_edit_status")->text().isEmpty());
        popup->findChild<QLineEdit*>(name)->setText("Updated " + resource);
        popup->findChild<QLineEdit*>(address)->setText("New address");
        popup->findChild<QLineEdit*>(prefix + "phone")->setText("0412 345 678");
        if (resource == "client") popup->findChild<QLineEdit*>(prefix + "email")->setText("client@example.invalid");
        else {
            auto* website = popup->findChild<QLineEdit*>(prefix + "website");
            website->setText("file:///invalid-website");
            popup->findChild<QPushButton*>(prefix + "save")->click();
            QCOMPARE(requests.size(), 0);
            website->setText("https://example.invalid");
            QVERIFY(popup->findChild<QCheckBox*>("form_gst_registered")->isChecked());
            auto* terms = popup->findChild<QComboBox*>("business_payment_terms");
            QCOMPARE(terms->currentData().toString(), "14");
            terms->setCurrentIndex(terms->findData("30"));
            QCOMPARE(popup->findChild<QLineEdit*>("BankName")->text(), "Original bank");
            popup->findChild<QLineEdit*>("BankName")->setText("Updated bank");
        }
        const auto preview = qEnvironmentVariable("INVOICE_RECORD_PREVIEW_DIR");
        if (!preview.isEmpty()) {
            QDir().mkpath(preview); QTest::qWait(50);
            QVERIFY(popup->grab().save(preview + "/" + resource + "-edit.png"));
        }
        popup->findChild<QPushButton*>(prefix + "save")->click();
        QCOMPARE(updates.size(), 1);
        QCOMPARE(requests.first()[0].toString(), resource);
        QTRY_VERIFY(popup.isNull());
        QCOMPARE(table->rowCount(), 1);
        QCOMPARE(table->item(0, 0)->data(Qt::UserRole), id);
        QCOMPARE(table->item(0, 0)->text(), "Updated " + resource);
        if (resource == "business") {
            const auto refreshed = qvariant_cast<QJsonDocument>(businesses.last().first()).object();
            const auto record = refreshed.begin().value().toObject();
            QCOMPARE(record.value("BusinessName").toString(), "Updated business");
            QCOMPARE(record.value("Address").toString(), "New address");
            QCOMPARE(record.value("InvoicePrefix").toString(), "KEEP-");
            QCOMPARE(record.value("InvoiceTerms").toString(), "Keep terms");
            QCOMPARE(record.value("PaymentTerms").toString(), "30");
            QCOMPARE(record.value("BankName").toString(), "Updated bank");
            QVERIFY(record.value("GSTRegistered").toBool());
        }
        // Double-click opens the same editor; cancelling keeps the saved values.
        emit table->cellDoubleClicked(0, 0);
        popup = page.findChild<QDialog*>(prefix + "dialog");
        QVERIFY(popup);
        popup->findChild<QLineEdit*>(name)->setText("Cancelled");
        popup->findChild<QPushButton*>(prefix + "cancel")->click();
        QTRY_VERIFY(popup.isNull());
        QCOMPARE(updates.size(), 1);
        QCOMPARE(table->item(0, 0)->text(), "Updated " + resource);
        tabs->setCurrentIndex(2);
        QCOMPARE(edit->text(), "Edit stock item");
        QVERIFY(!edit->isEnabled());
    }

    void record_updates_preserve_identity_and_business_isolation() {
        Application::Invoices::OfflineInvoiceSession session;
        const auto biz = session.save("business", {{"Business name", "Owner"}, {"InvoicePrefix", "KEEP-"}}, {});
        const auto other = session.save("business", {{"Business name", "Other"}}, {});
        const auto client = session.save("client", {{"Name", "Original"}}, biz);
        QJsonObject changes{{"_id", client}, {"Name", "Updated"}, {"Phone", ""}, {"Email", ""}, {"Address", ""},
            {"UserID", "attacker"}, {"BusinessID", other}};
        QVERIFY_EXCEPTION_THROWN(session.update_record(other, Domain::Records::RecordUpdate("client", changes)), std::invalid_argument);
        session.update_record(biz, Domain::Records::RecordUpdate("client", changes));
        const auto saved = session.clients.first().toObject();
        QCOMPARE(saved.value("_id").toString(), client);
        QCOMPARE(saved.value("BusinessID").toString(), biz);
        QVERIFY(!saved.contains("UserID"));
        changes["Name"] = "";
        QVERIFY_EXCEPTION_THROWN(session.update_record(biz, Domain::Records::RecordUpdate("client", changes)), std::invalid_argument);
        QVERIFY_EXCEPTION_THROWN(session.update_record(biz, Domain::Records::RecordUpdate("account", changes)), std::invalid_argument);
        QJsonObject business{{"_id", biz}, {"BusinessName", "New business"}, {"ABN", ""}, {"ACN", ""},
            {"Phone", ""}, {"BusinessAddress", ""}, {"Website", ""}, {"InvoicePrefix", "OVERWRITE"}};
        QVERIFY_EXCEPTION_THROWN(session.update_record(other, Domain::Records::RecordUpdate("business", business)), std::invalid_argument);
        session.update_record(biz, Domain::Records::RecordUpdate("business", business));
        QCOMPARE(session.businesses.value(biz).toObject().value("InvoicePrefix").toString(), "KEEP-");
    }

    void record_update_http_failure_retry_and_selection_change() {
        RecordingServer server; QVERIFY(server.start());
        QJsonObject received;
        bool fail = true;
        server.http.route("/client/update", QHttpServerRequest::Method::Post, [&](const QHttpServerRequest& request) {
            received = QJsonDocument::fromJson(request.body()).object();
            return fail ? QHttpServerResponse(QJsonObject{{"error", "Try again"}}, QHttpServerResponse::StatusCode::InternalServerError)
                        : QHttpServerResponse(QJsonObject{{"ok", true}});
        });
        ApiClient api(server.url(), nullptr);
        const QString biz = "65a000000000000000000001";
        api.business_selected({{"_id", biz}});
        QSignalSpy items(&api, &ApiClient::business_items_received);
        QSignalSpy saved(&api, &ApiClient::record_updated);
        QSignalSpy failed(&api, &ApiClient::record_update_failed);
        api.get_business_items(); QTRY_COMPARE(items.size(), 1);
        const QJsonObject changes{{"_id", "65b000000000000000000001"}, {"BusinessID", biz}, {"Name", "Updated"},
            {"Phone", ""}, {"Email", ""}, {"Address", ""}};
        api.update_record("client", changes, 1); QTRY_COMPARE(failed.size(), 1);
        QCOMPARE(received, changes); QCOMPARE(saved.size(), 0);
        fail = false;
        api.update_record("client", changes, 2); QTRY_COMPARE(saved.size(), 1);
        api.get_business_items(); QTRY_COMPARE(items.size(), 2);
        QCOMPARE(server.items_businesses.size(), 2);
        api.update_record("client", changes, 3);
        api.business_selected({{"_id", "different"}});
        QTest::qWait(80);
        QCOMPARE(saved.size(), 1);
    }

    void stock_records_popup_edits_offline_and_refreshes() {
        ApiClient api(QUrl("http://127.0.0.1:1"), nullptr);
        api.set_offline(true);
        QSignalSpy businesses(&api, &ApiClient::business_list_received);
        api.save_resource("business", QJsonDocument(QJsonObject{{"Business name", "Stock test"}}));
        api.get_business_list();
        const auto list = qvariant_cast<QJsonDocument>(businesses.last().first()).object();
        const auto business = list.begin().value().toObject();
        api.business_selected(business);
        api.save_resource("stock", QJsonDocument(QJsonObject{{"Name", "Original"}, {"Quantity", 10},
            {"Price", 12.5}, {"Margin", 0.25}, {"Unit", "box"}, {"Keywords", "one, two"}}));
        App::Views::ItemsPage page;
        page.business_selected(business);
        connect(&page, &App::Views::ItemsPage::items_requested, &api, &ApiClient::get_business_items);
        connect(&api, &ApiClient::business_items_received, &page, &App::Views::ItemsPage::populate_items);
        connect(&page, &App::Views::ItemsPage::record_update_requested, &api, &ApiClient::update_record);
        connect(&api, &ApiClient::record_updated, &page, &App::Views::ItemsPage::record_updated);
        connect(&api, &ApiClient::record_update_failed, &page, &App::Views::ItemsPage::record_update_failed);
        QSignalSpy updates(&api, &ApiClient::record_updated);
        QSignalSpy requests(&page, &App::Views::ItemsPage::record_update_requested);
        page.load_items();
        page.resize(900, 650); page.show();
        auto* table = page.findChild<QTableWidget*>("stock_records");
        page.findChild<QTabWidget*>("business_items_tabs")->setCurrentIndex(2);
        table->selectRow(0);
        const auto id = table->item(0, 0)->data(Qt::UserRole);
        page.findChild<QPushButton*>("edit_record")->click();
        QPointer<QDialog> popup = page.findChild<QDialog*>("stock_edit_dialog");
        QVERIFY(popup);
        QCOMPARE(popup->findChild<QLineEdit*>("stock_edit_name")->text(), "Original");
        QCOMPARE(popup->findChild<QLineEdit*>("stock_edit_margin")->text(), "25");
        popup->findChild<QLineEdit*>("stock_edit_name")->setText("Changed");
        popup->findChild<QLineEdit*>("stock_edit_quantity")->setText("2.5");
        popup->findChild<QPushButton*>("stock_edit_save")->click();
        QCOMPARE(requests.size(), 0);
        QVERIFY(!popup->findChild<QLabel*>("record_edit_status")->text().isEmpty());
        popup->findChild<QLineEdit*>("stock_edit_quantity")->setText("14");
        popup->findChild<QLineEdit*>("stock_edit_price")->setText("18.5");
        popup->findChild<QLineEdit*>("stock_edit_margin")->setText("30");
        popup->findChild<QLineEdit*>("stock_edit_keywords")->setText("Copper, pipe");
        const auto preview = qEnvironmentVariable("INVOICE_STOCK_PREVIEW_DIR");
        if (!preview.isEmpty()) {
            QDir().mkpath(preview); QTest::qWait(50);
            QVERIFY(popup->grab().save(preview + "/stock-edit.png"));
        }
        popup->findChild<QPushButton*>("stock_edit_save")->click();
        QCOMPARE(updates.size(), 1);
        QTRY_VERIFY(popup.isNull());
        QCOMPARE(table->rowCount(), 1);
        QCOMPARE(table->item(0, 0)->data(Qt::UserRole), id);
        QCOMPARE(table->item(0, 0)->text(), "Changed");
        QCOMPARE(table->item(0, 1)->text(), "14");
        QCOMPARE(table->item(0, 3)->text(), "30%");
        QCOMPARE(table->item(0, 5)->text(), "copper, pipe");
        table->selectRow(0);
        page.findChild<QPushButton*>("edit_record")->click();
        popup = page.findChild<QDialog*>("stock_edit_dialog");
        popup->findChild<QLineEdit*>("stock_edit_name")->setText("Cancelled");
        popup->findChild<QPushButton*>("stock_edit_cancel")->click();
        QTRY_VERIFY(popup.isNull());
        QCOMPARE(updates.size(), 1);
        QCOMPARE(table->item(0, 0)->text(), "Changed");
    }

    void stock_popup_retry_and_business_switch() {
        App::Views::ItemsPage page;
        const QString business = "65a000000000000000000001";
        page.business_selected({{"_id", business}});
        page.populate_items(business_items(business));
        page.findChild<QTabWidget*>("business_items_tabs")->setCurrentIndex(2);
        auto* table = page.findChild<QTableWidget*>("stock_records");
        table->selectRow(0);
        page.findChild<QPushButton*>("edit_record")->click();
        QPointer<QDialog> popup = page.findChild<QDialog*>("stock_edit_dialog");
        QSignalSpy saves(&page, &App::Views::ItemsPage::record_update_requested);
        popup->findChild<QLineEdit*>("stock_edit_name")->setText("Retry name");
        popup->findChild<QPushButton*>("stock_edit_save")->click();
        QCOMPARE(saves.size(), 1);
        const auto request = saves.last()[2].toULongLong();
        page.record_update_failed("Connection failed", request);
        QVERIFY(popup->isEnabled());
        QCOMPARE(popup->findChild<QLineEdit*>("stock_edit_name")->text(), "Retry name");
        popup->findChild<QPushButton*>("stock_edit_save")->click();
        QCOMPARE(saves.size(), 2);
        page.business_selected({{"_id", "different"}});
        QTRY_VERIFY(popup.isNull());
        page.record_updated(saves.last()[2].toULongLong());
        QCOMPARE(table->rowCount(), 0);
    }

    void stock_update_rejects_stale_quantity_and_other_business() {
        Application::Invoices::OfflineInvoiceSession session;
        const auto biz = session.save("business", {{"Business name", "Owner"}}, {});
        const auto other = session.save("business", {{"Business name", "Other"}}, {});
        const auto stock = session.save("stock", {{"Name", "Original"}, {"Price", 1}, {"Quantity", 4}}, biz);
        QJsonObject changes{{"_id", stock}, {"Name", "Updated"}, {"Quantity", -2}, {"ExpectedQuantity", 4},
            {"Price", 2}, {"Margin", 0.15}, {"Unit", ""}, {"Keywords", "A, B"}};
        QVERIFY_EXCEPTION_THROWN(session.update_record(other, Domain::Records::RecordUpdate("stock", changes)), std::invalid_argument);
        session.update_record(biz, Domain::Records::RecordUpdate("stock", changes));
        QCOMPARE(session.stocks.size(), 1);
        QCOMPARE(session.stocks.first().toObject().value("Quantity").toInt(), -2);
        QCOMPARE(session.stocks.first().toObject().value("Unit").toString(), "each");
        QVERIFY_EXCEPTION_THROWN(session.update_record(biz, Domain::Records::RecordUpdate("stock", changes)), std::invalid_argument);
        changes["ExpectedQuantity"] = -2;
        changes["Price"] = -1;
        QVERIFY_EXCEPTION_THROWN(session.update_record(biz, Domain::Records::RecordUpdate("stock", changes)), std::invalid_argument);
        QCOMPARE(session.stocks.first().toObject().value("Price").toDouble(), 2.0);
    }

    void stock_update_http_success_failure_and_cache_refresh() {
        RecordingServer server; QVERIFY(server.start());
        QJsonObject received;
        bool fail = true;
        server.http.route("/stock/update", QHttpServerRequest::Method::Post, [&](const QHttpServerRequest& request) {
            received = QJsonDocument::fromJson(request.body()).object();
            return fail ? QHttpServerResponse(QJsonObject{{"error", "Conflict"}}, QHttpServerResponse::StatusCode::Conflict)
                        : QHttpServerResponse(QJsonObject{{"ok", true}});
        });
        ApiClient api(server.url(), nullptr);
        const QString biz = "65a000000000000000000001";
        api.business_selected({{"_id", biz}});
        QSignalSpy items(&api, &ApiClient::business_items_received);
        QSignalSpy saved(&api, &ApiClient::record_updated);
        QSignalSpy failed(&api, &ApiClient::record_update_failed);
        api.get_business_items(); QTRY_COMPARE(items.size(), 1);
        const QJsonObject changes{{"_id", "65c000000000000000000001"}, {"BusinessID", biz}, {"Name", "Updated"},
            {"Quantity", 8}, {"ExpectedQuantity", 10}, {"Price", 10}, {"Margin", 0.2}, {"Unit", "each"}, {"Keywords", "a"}};
        api.update_record("stock", changes, 1); QTRY_COMPARE(failed.size(), 1);
        QCOMPARE(received, changes); QCOMPARE(saved.size(), 0);
        fail = false;
        api.update_record("stock", changes, 2); QTRY_COMPARE(saved.size(), 1);
        QCOMPARE(saved.first().first().toULongLong(), 2);
        api.get_business_items(); QTRY_COMPARE(items.size(), 2);
        QCOMPARE(server.items_businesses.size(), 2);
        api.update_record("stock", changes, 3);
        api.business_selected({{"_id", "different"}});
        QTest::qWait(80);
        QCOMPARE(saved.size(), 1);
    }

    void filename_template_pills_edit_drop_and_copy() {
        App::Views::DefaultInvoiceSettings settings;
        settings.set_business({{"_id", "filename-test"}, {"BusinessName", "Acme Plumbing"},
            {"InvoiceFilenameTemplate", "{business_name}_{invoice_number}"}});
        auto* editor = settings.findChild<QTextEdit*>("default_invoice_filename_template");
        auto* field = static_cast<App::Views::FilenameTemplateEdit*>(editor);
        QCOMPARE(field->pattern(), "{business_name}_{invoice_number}");
        QVERIFY(settings.findChild<QLabel*>("invoice_filename_preview")->text().contains("Acme_Plumbing_INV-000001.pdf"));
        field->set_pattern("");
        settings.findChild<QPushButton*>("filename_token_business_name")->click();
        QTest::keyClicks(field, "_");
        settings.findChild<QPushButton*>("filename_token_invoice_number")->click();
        QCOMPARE(field->pattern(), "{business_name}_{invoice_number}");
        QTest::keyClick(field, Qt::Key_Backspace);
        QCOMPARE(field->pattern(), "{business_name}_");
        field->undo();
        QCOMPARE(field->pattern(), "{business_name}_{invoice_number}");
        field->selectAll();
        field->copy();
        field->set_pattern("");
        field->paste();
        QCOMPARE(field->pattern(), "{business_name}_{invoice_number}");
        // A drop inserts at its position, rather than replacing the entire template.
        field->set_pattern("_suffix");
        field->resize(600, 68);
        field->show();
        QMimeData mime;
        mime.setData(App::Views::FilenameTemplateEdit::token_mime, "invoice_number");
        QDragEnterEvent enter(QPoint(2, 10), Qt::CopyAction, &mime, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(field->viewport(), &enter);
        QVERIFY(enter.isAccepted());
        QDropEvent drop(QPointF(2, 10), Qt::CopyAction, &mime, Qt::LeftButton, Qt::NoModifier);
        QApplication::sendEvent(field->viewport(), &drop);
        QVERIFY(drop.isAccepted());
        QCOMPARE(field->pattern(), "{invoice_number}_suffix");
        field->set_pattern("");
        settings.findChild<QPushButton*>("filename_token_invoice_number_only")->click();
        QCOMPARE(field->pattern(), "{invoice_number_only}");
        QVERIFY(settings.findChild<QLabel*>("invoice_filename_preview")->text().contains("000001.pdf"));
        QVERIFY(!settings.findChild<QLabel*>("invoice_filename_preview")->text().contains("INV-"));
        field->selectAll(); field->copy(); field->set_pattern(""); field->paste();
        QCOMPARE(field->pattern(), "{invoice_number_only}");
        field->set_pattern("{business_name}_{invoice_number_only}");
        const auto preview = qEnvironmentVariable("INVOICE_FILENAME_PREVIEW_DIR");
        if (!preview.isEmpty()) {
            QDir().mkpath(preview);
            settings.resize(900, 1000); settings.show(); QTest::qWait(40);
            QVERIFY(settings.grab().save(preview + "/filename-settings.png"));
        }
    }

    void filename_templates_persist_and_generate_offline() {
        ApiClient api(QUrl("http://127.0.0.1:1"), nullptr);
        api.set_offline(true);
        QSignalSpy lists(&api, &ApiClient::business_list_received);
        api.save_resource("business", QJsonDocument(QJsonObject{{"Business name", "Acme Plumbing"}}));
        api.get_business_list();
        const auto records = qvariant_cast<QJsonDocument>(lists.last().first()).object();
        auto business = records.begin().value().toObject();
        const auto id = business.value("_id").toString();
        QJsonObject settings{{"BusinessID", id}, {"InvoicePrefix", "JOB-"},
            {"InvoiceFilenameTemplate", "{business_name}_{invoice_number}"}};
        api.save_invoice_settings(settings);
        api.save_invoice_settings({{"BusinessID", id}, {"InvoicePrefix", "JOB-"}});
        business = qvariant_cast<QJsonDocument>(lists.last().first()).object().value(id).toObject();
        QCOMPARE(business.value("InvoiceFilenameTemplate").toString(), "{business_name}_{invoice_number}");
        App::Views::DefaultInvoiceSettings page;
        page.set_business(business, true);
        QSignalSpy saved(&page, &App::Views::DefaultInvoiceSettings::save_requested);
        page.findChild<QPushButton*>("save_invoice_defaults")->click();
        QCOMPARE(saved.size(), 1);
        QCOMPARE(saved.first().first().toJsonObject().value("InvoiceFilenameTemplate"), business.value("InvoiceFilenameTemplate"));
        page.save_failed("Retry");
        QVERIFY(page.findChild<QTextEdit*>("default_invoice_filename_template")->isEnabled());
        page.set_business({{"_id", "other"}}, true);
        QCOMPARE(static_cast<App::Views::FilenameTemplateEdit*>(page.findChild<QTextEdit*>("default_invoice_filename_template"))->pattern(), "{invoice_number}");
        QSignalSpy failed(&api, &ApiClient::invoice_settings_failed);
        for (const auto& invalid : {QJsonValue(12), QJsonValue("../file"), QJsonValue("{unknown}"), QJsonValue(QString(201, 'x'))}) {
            settings["InvoiceFilenameTemplate"] = invalid;
            api.save_invoice_settings(settings);
        }
        QCOMPARE(failed.size(), 4);
        Application::Invoices::OfflineInvoiceSession session;
        const auto biz = session.save("business", {{"Business name", "Acme Plumbing"}}, {});
        auto record = session.businesses[biz].toObject();
        record["InvoiceFilenameTemplate"] = "{business_name}_{invoice_number}.pdf";
        session.businesses[biz] = record;
        const auto client = session.save("client", {{"Name", "Customer"}}, biz);
        QTemporaryDir output;
        QJsonObject details{{"ClientID", client}, {"file_dir", output.path()}};
        const QJsonArray lines{QJsonObject{{"Name", "Service"}, {"Quantity", 1}, {"Price", 10}}};
        session.begin_invoice(details, biz);
        QCOMPARE(QFileInfo(session.generate_pdf(lines)).fileName(), "Acme_Plumbing_INV-000001.pdf");
        session.begin_invoice(details, biz);
        QCOMPARE(QFileInfo(session.generate_pdf(lines)).fileName(), "Acme_Plumbing_INV-000002.pdf");
        details["file_name"] = "my override";
        session.begin_invoice(details, biz);
        QCOMPARE(QFileInfo(session.generate_pdf(lines)).fileName(), "my_override.pdf");
        record = session.businesses[biz].toObject();
        record["InvoiceFilenameTemplate"] = "{business_name}_{invoice_number_only}";
        record["InvoicePrefix"] = "2026-";
        session.businesses[biz] = record;
        details.remove("file_name");
        session.begin_invoice(details, biz);
        QCOMPARE(session.invoice_number(), "2026-000004");
        QCOMPARE(QFileInfo(session.generate_pdf(lines)).fileName(), "Acme_Plumbing_000004.pdf");
        using namespace Domain::Invoices;
        QCOMPARE(invoice_filename("{invoice_number_only}", "", "2026000042", "2026"), "000042");
        QCOMPARE(invoice_filename("{invoice_number_only}", "", "000042", ""), "000042");
        QCOMPARE(invoice_filename("{business_name}_{invoice_number}", "Acme / Sydney", "INV:42"), "Acme_Sydney_INV_42");
        QCOMPARE(invoice_filename("", "Acme", "INV-42"), "INV-42");
        QCOMPARE(invoice_filename("CON.pdf", "", ""), "_CON");
        QVERIFY(!invoice_filename("{business_name}", QString(300, 'x'), "").isEmpty());
        QVERIFY(invoice_filename("{business_name}", QString(300, 'x'), "").size() <= 180);
    }

    void online_filename_uses_assigned_number_and_allows_override() {
        RecordingServer server; server.assigned_number = "JOB-000042";
        QVERIFY(server.start());
        TestAccountManager account;
        ApiClient api(server.url(), &account);
        const QJsonObject business{{"_id", "65a000000000000000000001"}, {"BusinessName", "Acme Plumbing"},
            {"InvoiceFilenameTemplate", "{business_name}_{invoice_number}"}};
        api.business_selected(business);
        App::Views::InvoiceDetailsInput page;
        page.business_selected(business);
        connect(&api, &ApiClient::invoice_number_assigned, &page, &App::Views::InvoiceDetailsInput::number_assigned);
        QTemporaryDir output;
        QSignalSpy started(&api, &ApiClient::invoice_started);
        QSignalSpy saved(&api, &ApiClient::pdf_generated);
        QJsonObject details{{"ClientID", "65b000000000000000000001"}, {"file_dir", output.path()}};
        const QJsonDocument lines(QJsonArray{QJsonObject{{"Name", "Service"}, {"Quantity", 1}, {"Price", 10}}});
        api.invoice_details(QJsonDocument(details));
        QTRY_COMPARE(started.size(), 1);
        QCOMPARE(page.findChild<QLineEdit*>("invoice_file_name")->text(), "Acme_Plumbing_JOB-000042");
        api.stock_list(lines);
        QTRY_COMPARE(saved.size(), 1);
        QCOMPARE(QFileInfo(saved.first().first().toString()).fileName(), "Acme_Plumbing_JOB-000042.pdf");
        details["file_name"] = "custom";
        api.invoice_details(QJsonDocument(details)); QTRY_COMPARE(started.size(), 2);
        api.stock_list(lines); QTRY_COMPARE(saved.size(), 2);
        QCOMPARE(QFileInfo(saved.last().first().toString()).fileName(), "custom.pdf");
        auto numbers_only = business;
        numbers_only["InvoicePrefix"] = "JOB-";
        numbers_only["InvoiceFilenameTemplate"] = "{invoice_number_only}";
        api.business_selected(numbers_only);
        page.business_selected(numbers_only);
        page.prepare_invoice("JOB-", output.path());
        details.remove("file_name");
        api.invoice_details(QJsonDocument(details)); QTRY_COMPARE(started.size(), 3);
        QCOMPARE(page.findChild<QLineEdit*>("invoice_file_name")->text(), "000042");
        api.stock_list(lines); QTRY_COMPARE(saved.size(), 3);
        QCOMPARE(QFileInfo(saved.last().first().toString()).fileName(), "000042.pdf");
        QVERIFY(!server.invoices.first().contains("file_name"));
        QVERIFY(!server.invoices.first().contains("file_dir"));
    }

    void illegal_filenames_are_rejected_before_starting_invoices() {
        using namespace Domain::Invoices;
        RecordingServer server;
        QVERIFY(server.start());
        TestAccountManager account;
        ApiClient api(server.url(), &account);
        api.business_selected({{"_id", "65a000000000000000000001"}});
        QSignalSpy failed(&api, &ApiClient::invoice_failed);
        Application::Invoices::OfflineInvoiceSession session;
        const auto biz = session.save("business", {{"Business name", "Acme"}}, {});
        const auto client = session.save("client", {{"Name", "Customer"}}, biz);
        QTemporaryDir output;
        const QStringList invalid{"CON", "nul.pdf", "COM1.extra.pdf", "LPT9", QString::fromUtf8("COM\xc2\xb9.pdf"),
            "../file", "a/b", "a\\b", "a:b", "a<b", "a>b", "a\"b", "a|b", "a?b", "a*b",
            "a\nb", QString("a") + QChar(0) + "b", ".", "..", ".pdf", "invoice.", "invoice ", "   ",
            QString(181, 'x'), QString(100, QChar(0x4e00))};
        for (const auto& name : invalid) {
            QVERIFY2(!filename_error(name).isEmpty(), qPrintable(name));
            QVERIFY2(!filename_template_error(name).isEmpty(), qPrintable(name));
            QJsonObject details{{"ClientID", client}, {"file_dir", output.path()}, {"file_name", name}};
            QVERIFY_EXCEPTION_THROWN(session.begin_invoice(details, biz), std::runtime_error);
            api.invoice_details(QJsonDocument(details));
        }
        QCOMPARE(failed.size(), invalid.size());
        QVERIFY(server.invoices.isEmpty());
        QCOMPARE(session.businesses[biz].toObject().value("InvoiceSequence").toInteger(), 0);
        for (const auto& name : {QString("invoice"), QString("my invoice.PDF"), QString("CON_report"), QString("COM10")})
            QVERIFY2(filename_error(name).isEmpty(), qPrintable(name));
        for (const auto& name : {QString("CON"), QString("a/b"), QString(300, QChar(0x4e00)), QString(300, 'x')})
            QVERIFY(filename_error(invoice_filename("{business_name}", name, "")).isEmpty());
    }

    void business_gst_payment_preferences_and_rounding() {
        Application::Invoices::OfflineInvoiceSession session;
        const auto business = session.save("business", {{"Business name", "GST business"},
            {"Are You Registered for GST", true}, {"PaymentTerms", "30"},
            {"BankName", "Example bank"}, {"BankAccountName", "Example business"},
            {"BankBSB", "012-345"}, {"BankAccountNumber", "00123456"}, {"InvoiceTerms", "Original terms"}}, {});
        const auto client = session.save("client", {{"Name", "Customer"}}, business);
        QTemporaryDir output;
        session.begin_invoice({{"ClientID", client}, {"file_dir", output.path()},
            {"date_created", "2026-12-20"}, {"date_due", "2026-12-21"}}, business);
        const QJsonArray lines{QJsonObject{{"Name", "Service"}, {"Quantity", 2}, {"Price", 19.99}},
            QJsonObject{{"Name", "Small line"}, {"Quantity", 1}, {"Price", 0.05}},
            QJsonObject{{"Name", "Another small line"}, {"Quantity", 1}, {"Price", 0.05}}};
        QVERIFY(QFile::exists(session.generate_pdf(lines)));
        auto record = session.invoices.first().toObject();
        QCOMPARE(record.value("DateDue").toString(), "2027-01-19");
        QCOMPARE(record.value("Subtotal").toDouble(), 40.08);
        QCOMPARE(record.value("GST").toDouble(), 4.02);
        QCOMPARE(record.value("Total").toDouble(), 44.10);
        QCOMPARE(record.value("Items").toArray(), lines);
        auto changed = session.businesses[business].toObject();
        changed["GSTRegistered"] = false;
        changed["BankBSB"] = "999-999";
        changed["PaymentTerms"] = "COD";
        session.businesses[business] = changed;
        const auto restored = Domain::Invoices::restore(record);
        QVERIFY(restored->getBusiness()->get_gst_registered());
        QCOMPARE(restored->getBusiness()->get_bank_bsb(), std::string("012-345"));
        QCOMPARE(restored->getBusiness()->get_bank_account_number(), std::string("00123456"));
        QCOMPARE(restored->getBusiness()->get_invoice_terms(), std::string("Original terms"));
        QVERIFY(Infrastructure::PDF::InvoicePdfGenerator::render_record(record).startsWith("%PDF-"));
        session.begin_invoice({{"ClientID", client}, {"file_dir", output.path()}, {"date_created", "2026-12-20"}}, business);
        session.generate_pdf(lines);
        record = session.invoices.last().toObject();
        QCOMPARE(record.value("GST").toDouble(), 0.0);
        QCOMPARE(record.value("Total").toDouble(), 40.08);
        QCOMPARE(record.value("DateDue").toString(), "2026-12-20");
        Domain::Invoices::Amounts large;
        large.add(123456.78, 3, true);
        QCOMPARE(large.subtotal, 370370.34);
        QCOMPARE(large.gst, 37037.03);
        Domain::Invoices::Amounts half_cent;
        half_cent.add(1.15, 1, true);
        QCOMPARE(half_cent.gst, 0.12);
    }

    void payment_dates_follow_business_and_issue_date() {
        App::Views::InvoiceDetailsInput page;
        auto* issued = page.findChild<QDateEdit*>("date_created");
        auto* due = page.findChild<QDateEdit*>("date_due");
        QVERIFY(issued && due && due->isReadOnly());
        issued->setDate(QDate(2028, 2, 20));
        page.business_selected({{"_id", "A"}, {"PaymentTerms", "14"}});
        QCOMPARE(due->date(), QDate(2028, 3, 5));
        issued->setDate(QDate(2026, 12, 20));
        QCOMPARE(due->date(), QDate(2027, 1, 3));
        page.business_selected({{"_id", "B"}, {"PaymentTerms", "30"}});
        QCOMPARE(due->date(), QDate(2027, 1, 19));
        page.business_selected({{"_id", "C"}});
        QCOMPARE(due->date(), issued->date());
    }

    void invoice_preferences_save_reload_and_validate() {
        ApiClient api(QUrl("http://127.0.0.1:1"), nullptr);
        api.set_offline(true);
        QSignalSpy businesses(&api, &ApiClient::business_list_received);
        api.save_resource("business", QJsonDocument(QJsonObject{{"Business name", "Preferences"}}));
        api.get_business_list();
        const auto records = qvariant_cast<QJsonDocument>(businesses.last().first()).object();
        auto business = records.begin().value().toObject();
        const auto id = business.value("_id").toString();
        App::Views::DefaultInvoiceSettings settings;
        settings.set_business(business, true);
        settings.findChild<QCheckBox*>("settings_gst_registered")->setChecked(true);
        settings.findChild<QComboBox*>("business_payment_terms")->setCurrentIndex(2);
        settings.findChild<QLineEdit*>("BankBSB")->setText("012-345");
        settings.findChild<QLineEdit*>("BankAccountNumber")->setText("00123456");
        connect(&settings, &App::Views::DefaultInvoiceSettings::save_requested, &api, &ApiClient::save_invoice_settings);
        connect(&api, &ApiClient::invoice_settings_saved, &settings, &App::Views::DefaultInvoiceSettings::save_succeeded);
        settings.findChild<QPushButton*>("save_invoice_defaults")->click();
        business = qvariant_cast<QJsonDocument>(businesses.last().first()).object().value(id).toObject();
        QVERIFY(business.value("GSTRegistered").toBool());
        QCOMPARE(business.value("PaymentTerms").toString(), "30");
        QCOMPARE(business.value("BankAccountNumber").toString(), "00123456");
        api.save_invoice_settings({{"BusinessID", id}, {"InvoicePrefix", "INV-"}});
        QCOMPARE(qvariant_cast<QJsonDocument>(businesses.last().first()).object().value(id).toObject().value("BankBSB").toString(), "012-345");
        settings.set_business({{"_id", "Other"}}, true);
        QVERIFY(!settings.findChild<QCheckBox*>("settings_gst_registered")->isChecked());
        QVERIFY(settings.findChild<QLineEdit*>("BankBSB")->text().isEmpty());
        settings.set_business(business, true);
        QCOMPARE(settings.findChild<QComboBox*>("business_payment_terms")->currentData().toString(), "30");
        QSignalSpy failed(&api, &ApiClient::invoice_settings_failed);
        for (const auto& bad : {QJsonObject{{"PaymentTerms", "7"}}, QJsonObject{{"PaymentTerms", 14}},
             QJsonObject{{"GSTRegistered", "yes"}}, QJsonObject{{"BankBSB", 123}},
             QJsonObject{{"BankName", QString(121, 'x')}}}) {
            auto invalid = bad; invalid["BusinessID"] = id; invalid["InvoicePrefix"] = "INV-";
            api.save_invoice_settings(invalid);
        }
        QCOMPARE(failed.size(), 5);
        settings.findChild<QLineEdit*>("BankBSB")->clear();
        settings.findChild<QPushButton*>("save_invoice_defaults")->click();
        QVERIFY(qvariant_cast<QJsonDocument>(businesses.last().first()).object().value(id).toObject().value("BankBSB").toString().isEmpty());
    }
    void account_settings_populates_current_identity_on_each_visit() {
        TestAccountManager account;
        App::Views::MainWindow window(account);
        auto* form = window.account_settings_page();
        const auto fields = form->findChildren<QLineEdit*>("form_input");
        QCOMPARE(fields.size(), 4);

        // The window and form are constructed before sign-in completes.
        auto first_user = std::make_shared<User>("alex@example.com", "never display this");
        first_user->setFirstName("Alex");
        first_user->setLastName("Taylor");
        account.set_authenticated_user(first_user);
        window.show_page(form);
        QCOMPARE(fields[0]->text(), QString("Alex"));
        QCOMPARE(fields[1]->text(), QString("Taylor"));
        QCOMPARE(fields[2]->text(), QString("alex@example.com"));
        QVERIFY(fields[3]->text().isEmpty());
        QCOMPARE(fields[3]->echoMode(), QLineEdit::Password);

        fields[0]->setText("Unsaved edit");
        fields[3]->setText("Unsaved password");
        window.show_page(window.dashboard_page());
        first_user->setLastName("Updated");
        window.show_page(form);
        QCOMPARE(fields[0]->text(), QString("Alex"));
        QCOMPARE(fields[1]->text(), QString("Updated"));
        QVERIFY(fields[3]->text().isEmpty());

        // A different or signed-out user must never see the previous profile.
        account.set_authenticated_user(std::make_shared<User>("second@example.com", ""));
        window.show_page(form);
        QVERIFY(fields[0]->text().isEmpty());
        QVERIFY(fields[1]->text().isEmpty());
        QCOMPARE(fields[2]->text(), QString("second@example.com"));
        account.logOut();
        window.show_page(form);
        for (const auto* field : fields) QVERIFY(field->text().isEmpty());
    }

    void online_finish_downloads_pdf_and_retries_failures() {
        RecordingServer server;
        QVERIFY(server.start());
        TestAccountManager account;
        ApiClient api(server.url(), &account);
        App::Views::MainWindow window(account);
        Application::Controllers::AppController controller(&window, account, &api);
        QTemporaryDir output;
        QVERIFY(output.isValid());
        api.business_selected(QJsonObject{{"_id", "65a000000000000000000001"}});
        QSignalSpy started(&api, &ApiClient::invoice_started);
        QSignalSpy saved(&api, &ApiClient::pdf_generated);
        QSignalSpy failed(&api, &ApiClient::pdf_failed);
        auto details = QJsonObject{{"ClientID", "65b000000000000000000001"}, {"invoice_number", "TEST-001"},
                                  {"file_dir", output.path()}, {"file_name", "remote invoice"}};
        api.invoice_details(QJsonDocument(details));
        QTRY_COMPARE(started.count(), 1);
        QVERIFY(!server.invoices.last().contains("file_dir"));
        QVERIFY(!server.invoices.last().contains("file_name"));
        auto* page = window.new_invoice_stock_page();
        auto* finish = page->findChild<QPushButton*>("invoice_finish");
        QVERIFY(finish);
        finish->click();
        QVERIFY(page->findChild<QLabel*>("invoice_stock_status")->text().contains("at least one item"));
        QCOMPARE(server.pdf_requests, 0);
        page->stock_items = QJsonArray{QJsonObject{{"Name", "Service"}, {"Quantity", 2}, {"Price", 15.5}}};
        finish->click();
        QVERIFY(!finish->isEnabled());
        QTRY_COMPARE(saved.count(), 1);
        QVERIFY(finish->isEnabled());
        const auto path = output.filePath("remote_invoice.pdf");
        QCOMPARE(saved.last().first().toString(), path);
        const auto read_pdf = [&]() {
            QFile file(path);
            return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray{};
        };
        const auto expected = server.pdf_body;
        QCOMPARE(read_pdf(), expected);
        QVERIFY(page->findChild<QLabel*>("invoice_stock_status")->text().contains(path));

        server.pdf_type = "application/json";
        server.pdf_status = 500;
        server.pdf_body = R"({"error":"Could not generate the invoice PDF."})";
        finish->click();
        QTRY_COMPARE(failed.count(), 1);
        QVERIFY(finish->isEnabled());
        QCOMPARE(read_pdf(), expected); // An error must not replace the existing file.
        QVERIFY(page->findChild<QLabel*>("invoice_stock_status")->text().contains("Could not generate"));

        server.pdf_status = 200; // Older backend returns text instead of a PDF.
        finish->click();
        QTRY_COMPARE(failed.count(), 2);
        QCOMPARE(read_pdf(), expected);
        server.pdf_type = "application/pdf";
        server.pdf_body = expected;
        finish->click();
        QTRY_COMPARE(saved.count(), 2);

        // A local write failure is reported and leaves the form available to retry.
        details["file_name"] = "blocked.pdf";
        QVERIFY(QDir(output.path()).mkdir("blocked.pdf"));
        api.invoice_details(QJsonDocument(details));
        QTRY_COMPARE(started.count(), 2);
        page->stock_items = QJsonArray{QJsonObject{{"Name", "Service"}, {"Quantity", 2}, {"Price", 15.5}}};
        finish->click();
        QTRY_COMPARE(failed.count(), 3);
        QVERIFY(finish->isEnabled());
        QVERIFY(failed.last().first().toString().contains("Could not save PDF"));

        // Invalid names now fail before reserving a number or reaching the export step.
        details["file_name"] = "../escape";
        QSignalSpy start_failed(&api, &ApiClient::invoice_failed);
        QTimer::singleShot(0, &window, []() {
            if (auto* dialog = qobject_cast<QMessageBox*>(QApplication::activeModalWidget())) dialog->accept();
        });
        api.invoice_details(QJsonDocument(details));
        QCOMPARE(start_failed.count(), 1);
        QCOMPARE(started.count(), 2);
        QCOMPARE(server.invoices.size(), 2);
        const auto requests = server.pdf_requests;

        // Switching business while a download is pending must not save the old invoice.
        details["file_name"] = "stale";
        api.invoice_details(QJsonDocument(details));
        QTRY_COMPARE(started.count(), 3);
        api.stock_list(QJsonDocument(page->stock_items));
        api.business_selected(QJsonObject{{"_id", "65a000000000000000000002"}});
        QTRY_COMPARE(server.pdf_requests, requests + 1);
        QTest::qWait(100);
        QVERIFY(!QFileInfo::exists(output.filePath("stale.pdf")));
        QCOMPARE(saved.count(), 2);
    }

    void online_stock_shortage_choices_data() {
        QTest::addColumn<QString>("policy");
        QTest::newRow("zero") << QString("zero");
        QTest::newRow("negative") << QString("negative");
    }

    void online_stock_shortage_choices() {
        QFETCH(QString, policy);
        RecordingServer server;
        QVERIFY(server.start());
        TestAccountManager account;
        ApiClient api(server.url(), &account);
        App::Views::MainWindow window(account);
        Application::Controllers::AppController controller(&window, account, &api);
        QTemporaryDir output;
        QVERIFY(output.isValid());
        api.business_selected(QJsonObject{{"_id", "65a000000000000000000001"}});
        QSignalSpy started(&api, &ApiClient::invoice_started);
        QSignalSpy saved(&api, &ApiClient::pdf_generated);
        QSignalSpy failed(&api, &ApiClient::pdf_failed);
        api.invoice_details(QJsonDocument(QJsonObject{{"ClientID", "65b000000000000000000001"},
            {"file_dir", output.path()}, {"file_name", "shortage"}}));
        QTRY_COMPARE(started.count(), 1);
        auto* page = window.new_invoice_stock_page();
        auto* finish = page->findChild<QPushButton*>("invoice_finish");
        const QJsonArray lines{QJsonObject{{"_id", "65c000000000000000000001"},
            {"Name", "Widget"}, {"Quantity", 4}, {"Price", 10}}};
        page->stock_items = lines;
        const auto pdf = server.pdf_body;
        server.pdf_type = "application/json";
        server.pdf_status = 409;
        server.pdf_body = R"({"error":"Insufficient stock","code":"stock_shortage","items":[{"Name":"Widget","Available":2,"Requested":4}]})";
        finish->click();
        QTRY_VERIFY(page->findChild<QMessageBox*>("stock_shortage_dialog"));
        QPointer<QMessageBox> dialog = page->findChild<QMessageBox*>("stock_shortage_dialog");
        QVERIFY(dialog->informativeText().contains("2 available, 4 on this invoice"));
        QVERIFY(!finish->isEnabled());
        QCOMPARE(server.pdf_payloads.last().array(), lines);
        QVERIFY(failed.isEmpty());
        QVERIFY(saved.isEmpty());
        QVERIFY(!QFile::exists(output.filePath("shortage.pdf")));
        dialog->findChild<QPushButton*>("shortage_cancel")->click();
        QTRY_VERIFY(dialog.isNull());
        QVERIFY(finish->isEnabled());
        QCOMPARE(server.pdf_requests, 1);
        QCOMPARE(page->stock_items, lines);

        // Dismissing the prompt must not carry a choice into the next attempt.
        finish->click();
        QTRY_VERIFY(page->findChild<QMessageBox*>("stock_shortage_dialog"));
        dialog = page->findChild<QMessageBox*>("stock_shortage_dialog");
        QCOMPARE(server.pdf_payloads.last().array(), lines);
        server.pdf_type = "application/pdf";
        server.pdf_status = 200;
        server.pdf_body = pdf;
        // Simulate a local write failure after the backend has completed the invoice.
        QVERIFY(QDir(output.path()).mkdir("shortage.pdf"));
        dialog->findChild<QPushButton*>("shortage_" + policy)->click();
        QTRY_COMPARE(failed.count(), 1);
        QCOMPARE(server.pdf_payloads.last().object().value("stockShortagePolicy").toString(), policy);
        QCOMPARE(server.pdf_payloads.last().object().value("items").toArray(), lines);
        QVERIFY(finish->isEnabled());
        QVERIFY(QDir(output.path()).rmdir("shortage.pdf"));
        finish->click();
        QTRY_COMPARE(saved.count(), 1);
        QCOMPARE(server.pdf_payloads.last().object().value("stockShortagePolicy").toString(), policy);
        QFile file(output.filePath("shortage.pdf"));
        QVERIFY(file.open(QIODevice::ReadOnly));
        QCOMPARE(file.readAll(), pdf);
        file.close();

        // Starting a new invoice closes a pending prompt without submitting it.
        page->reset_invoice();
        page->stock_items = lines;
        server.pdf_type = "application/json";
        server.pdf_status = 409;
        server.pdf_body = R"({"code":"stock_shortage","items":[{"Name":"Widget","Available":0,"Requested":4}]})";
        finish->click();
        QTRY_VERIFY(page->findChild<QMessageBox*>("stock_shortage_dialog"));
        dialog = page->findChild<QMessageBox*>("stock_shortage_dialog");
        QCOMPARE(server.pdf_payloads.last().array(), lines);
        const auto requests = server.pdf_requests;
        page->reset_invoice();
        QTRY_VERIFY(dialog.isNull());
        QCOMPARE(server.pdf_requests, requests);
        QVERIFY(page->stock_items.isEmpty());
        QVERIFY(finish->isEnabled());
    }

    void auth_sessions_isolate_users_and_expire() {
        Infrastructure::Http::AuthSessions sessions;
        const auto now = QDateTime::currentDateTimeUtc();
        const auto first = sessions.create("65d000000000000000000001", now);
        const auto second = sessions.create("65d000000000000000000002", now);
        QVERIFY(!first.isEmpty());
        QVERIFY(first != second);
        QCOMPARE(sessions.find(first, now)->user_id, std::string("65d000000000000000000001"));
        QCOMPARE(sessions.find(second, now)->user_id, std::string("65d000000000000000000002"));
        QVERIFY(!sessions.find("invalid", now));
        sessions.revoke(first);
        QVERIFY(!sessions.find(first, now));
        QVERIFY(sessions.find(second, now));
        QVERIFY(!sessions.find(second, now.addSecs(8 * 60 * 60)));
    }

    void remote_login_carries_session_and_reports_errors() {
        QHttpServer http;
        auto* tcp = new QTcpServer(&http);
        int status = 200;
        QByteArray body = R"({"ok":true,"token":"test-session-token","user":{"_id":"65d000000000000000000001","email":"alice@example.invalid"}})";
        QByteArray business_auth;
        QByteArray logout_auth;
        http.route("/auth/login", QHttpServerRequest::Method::Post, [&]() {
            return QHttpServerResponse("application/json", body, static_cast<QHttpServerResponder::StatusCode>(status));
        });
        http.route("/business/list", QHttpServerRequest::Method::Get, [&](const QHttpServerRequest& request) {
            business_auth = request.value("Authorization");
            return QHttpServerResponse(QJsonObject{});
        });
        http.route("/auth/logout", QHttpServerRequest::Method::Post, [&](const QHttpServerRequest& request) {
            logout_auth = request.value("Authorization");
            return QHttpServerResponse(QJsonObject{{"ok", true}});
        });
        QVERIFY(tcp->listen(QHostAddress::LocalHost, 0));
        QVERIFY(http.bind(tcp));
        Application::Accounts::FrontendAccountSession account;
        ApiClient api(QUrl(QString("http://127.0.0.1:%1").arg(tcp->serverPort())), &account);
        QSignalSpy success(&api, &ApiClient::login_succeeded);
        QSignalSpy failure(&api, &ApiClient::login_failed);
        api.do_login("alice@example.invalid", "test-password", false);
        QTRY_COMPARE(success.count(), 1);
        QVERIFY(account.is_logged_in());
        QCOMPARE(account.getAccount()->getMongoUserID(), std::string("65d000000000000000000001"));
        QVERIFY(account.getAccount()->getPassword().empty());
        api.get_business_list();
        QTRY_COMPARE(business_auth, QByteArray("Bearer test-session-token"));
        api.clear_session();
        QVERIFY(!account.is_logged_in());
        QTRY_COMPARE(logout_auth, QByteArray("Bearer test-session-token"));

        status = 401;
        body = R"({"error":"Invalid email or password"})";
        api.do_login("alice@example.invalid", "incorrect", false);
        QTRY_COMPARE(failure.count(), 1);
        QCOMPARE(failure.takeFirst().at(0).toString(), QString("Invalid email or password"));
        status = 503;
        body = R"({"error":"The server could not connect to the database."})";
        api.do_login("alice@example.invalid", "test-password", false);
        QTRY_COMPARE(failure.count(), 1);
        QCOMPARE(failure.takeFirst().at(0).toString(), QString("The server could not connect to the database."));
        status = 200;
        body = R"({"ok":true})";
        api.do_login("alice@example.invalid", "test-password", false);
        QTRY_COMPARE(failure.count(), 1);
        QVERIFY(failure.takeFirst().at(0).toString().contains("invalid sign-in response"));
        QCOMPARE(success.count(), 1);
        QVERIFY(!account.is_logged_in());
    }

    void initTestCase() {
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, settings_directory_.path());
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

    void offline_invoice_reuses_forms_and_exports_without_network() {
        RecordingServer server;
        QVERIFY(server.start());
        QSignalSpy connections(server.tcp, &QTcpServer::newConnection);
        TestAccountManager account;
        ApiClient api(server.url(), &account);
        App::Views::MainWindow window(account);
        Application::Controllers::AppController controller(&window, account, &api);
        window.resize(800, 800);
        window.show();
        auto* stack = window.findChild<QStackedWidget*>();
        window.login_page()->findChild<QPushButton*>("offline_invoice_button")->click();
        QVERIFY(api.is_offline());
        QVERIFY(!account.is_logged_in());
        QCOMPARE(stack->currentWidget(), window.business_settings_page());
        QVERIFY(window.findChild<QWidget*>("offline_banner")->isVisible());
        QVERIFY(button(*window.dashboard_page(), "Account settings")->isHidden());
        QCoreApplication::processEvents();
        QVERIFY(window.grab().save("offline-business-ui.png"));

        const auto fill = [](App::Views::ManagementForm* form, const QMap<QString, QString>& values) {
            for (auto* field : form->findChildren<QLineEdit*>()) {
                if (values.contains(field->placeholderText()))
                    field->setText(values.value(field->placeholderText()));
            }
            form->findChild<QPushButton*>("register_button_")->click();
        };
        const QMap<QString, QString> address{{"Country", "Australia"}, {"State or province", "NSW"},
            {"City", "Sydney"}, {"Street address", "10 Demo Street"}, {"Postcode", "2000"}};
        auto business = address;
        business.insert("ABN", "12345678901");
        business.insert("ACN", "123456789");
        business.insert("Business name", "Demo business");
        business.insert("Business phone", "0212345678");
        fill(window.business_settings_page(), business);
        QCOMPARE(stack->currentWidget(), window.dashboard_page());
        QVERIFY(window.dashboard_page()->has_business());
        QCoreApplication::processEvents();
        QVERIFY(window.findChild<QWidget*>("offline_banner")->isVisible());
        QVERIFY(window.findChild<QPushButton*>("exit_offline_button")->isVisible());
        QVERIFY(window.grab().save("offline-dashboard-ui.png"));
        controller.page_navigation(Page::AccountSettings);
        QCOMPARE(stack->currentWidget(), window.dashboard_page());
        controller.page_navigation(Page::NewClient);
        auto client = address;
        client.insert("Name", "Demo client");
        client.insert("Phone", "0212345678");
        client.insert("Email", "demo@example.com");
        fill(window.client_page(), client);
        QCOMPARE(stack->currentWidget(), window.dashboard_page());
        controller.page_navigation(Page::StockSettings);
        fill(window.stock_settings_page(), {{"Name", "Demo service"}, {"Quantity", "10"}, {"Price", "25"},
            {"Margin", "0"}, {"Keywords", "demo"}, {"Unit", "hour"}});
        QCOMPARE(stack->currentWidget(), window.dashboard_page());

        QSignalSpy records(&api, &ApiClient::business_items_received);
        api.get_business_items();
        const auto items = qvariant_cast<QJsonDocument>(records.last().at(0)).object();
        QCOMPARE(items.value("clients").toArray().size(), 1);
        QVERIFY(items.value("clients").toArray().first().toObject().value("Address").toString().contains("10 Demo Street"));
        QCOMPARE(items.value("stocks").toArray().size(), 1);
        QCOMPARE(items.value("stocks").toArray().first().toObject().value("Price").toDouble(), 25.0);
        controller.page_navigation(Page::NewInvoice);
        auto* details = window.new_invoice_page();
        auto* clients = details->findChild<QComboBox*>("invoice_client");
        QCOMPARE(clients->count(), 2);
        clients->setCurrentIndex(1);
        QTemporaryDir output;
        QVERIFY(output.isValid());
        details->findChild<QLineEdit*>("invoice_number")->setText("DEMO-001");
        details->findChild<QLineEdit*>("invoice_file_name")->setText("demo invoice");
        details->findChild<QLineEdit*>("invoice_file_dir")->setText(output.path());
        details->findChild<QPushButton*>("invoice_next")->click();
        QCOMPARE(stack->currentWidget(), window.new_invoice_stock_page());
        auto* stock = window.new_invoice_stock_page();
        QCOMPARE(stock->findChild<QComboBox*>("invoice_saved_stock")->count(), 1);
        stock->findChild<QLineEdit*>("invoice_new_stock_name")->setText("New demo item");
        qobject_cast<QLineEdit*>(stock->invoice_body_form_fields.value("quantity"))->setText("2");
        qobject_cast<QLineEdit*>(stock->invoice_body_form_fields.value("price"))->setText("15.50");
        stock->findChild<QPushButton*>("invoice_add_item")->click();
        QCOMPARE(stock->stock_items.size(), 1);
        QSignalSpy pdf(&api, &ApiClient::pdf_generated);
        QPushButton* finish = nullptr;
        for (auto* candidate : stock->findChildren<QPushButton*>())
            if (candidate->text() == "Finish invoice") finish = candidate;
        QVERIFY(finish);
        finish->click();
        QCOMPARE(pdf.count(), 1);
        QFile file(output.filePath("demo_invoice.pdf"));
        QVERIFY(file.open(QIODevice::ReadOnly));
        QCOMPARE(file.read(5), QByteArray("%PDF-"));
        QVERIFY(stock->findChild<QLabel*>("invoice_stock_status")->text().contains("PDF saved"));
        QCoreApplication::processEvents();
        QVERIFY(window.grab().save("offline-invoice-ui.png"));
        controller.page_navigation(Page::Dashboard);
        window.dashboard_page()->findChild<QPushButton*>("browse_invoices_button")->click();
        QCOMPARE(stack->currentWidget(), window.invoices_page());
        auto* history = window.invoices_page()->findChild<QTableWidget*>("invoice_records");
        QCOMPARE(history->rowCount(), 1);
        history->selectRow(0);
        window.invoices_page()->findChild<QPushButton*>("invoice_payment")->click();
        QCOMPARE(history->item(0, 5)->text(), "Paid");
        QSignalSpy exports(&api, &ApiClient::invoice_exported);
        QSignalSpy invoices(&api, &ApiClient::invoices_received);
        api.get_invoices();
        const auto id = qvariant_cast<QJsonDocument>(invoices.last().first()).object().value("invoices").toArray().first().toObject().value("_id").toString();
        api.export_saved_invoice(id, output.filePath("archived.pdf"));
        QCOMPARE(exports.count(), 1);
        QVERIFY(QFile::exists(output.filePath("archived.pdf")));
        QTest::qWait(100);
        QCOMPARE(connections.count(), 0);
        QVERIFY(server.saves.isEmpty());
        QVERIFY(server.invoices.isEmpty());

        window.findChild<QPushButton*>("exit_offline_button")->click();
        QVERIFY(!api.is_offline());
        QCOMPARE(stack->currentWidget(), window.landing_page());
        QVERIFY(window.findChild<QWidget*>("offline_banner")->isHidden());
        QVERIFY(!window.dashboard_page()->has_business());
        QVERIFY(stock->stock_items.isEmpty());
        QVERIFY(details->findChild<QLineEdit*>("invoice_number")->text().isEmpty());
        window.landing_page()->findChild<QPushButton*>("offline_invoice_button")->click();
        QVERIFY(!window.dashboard_page()->has_business());
        api.get_business_items();
        const auto empty = qvariant_cast<QJsonDocument>(records.last().at(0)).object();
        QVERIFY(empty.value("clients").toArray().isEmpty());
        QVERIFY(empty.value("stocks").toArray().isEmpty());
    }

    void offline_api_blocks_accounts_and_online_records_and_handles_pdf_failure() {
        RecordingServer server;
        QVERIFY(server.start());
        QSignalSpy connections(server.tcp, &QTcpServer::newConnection);
        ApiClient api(server.url(), nullptr);
        api.set_offline(true);
        QSignalSpy saved(&api, &ApiClient::resource_saved);
        QSignalSpy failed(&api, &ApiClient::resource_save_failed);
        QSignalSpy signup(&api, &ApiClient::signup_failed);
        QSignalSpy login(&api, &ApiClient::login_failed);
        QSignalSpy invoice_error(&api, &ApiClient::invoice_failed);
        QSignalSpy pdf_error(&api, &ApiClient::pdf_failed);
        QSignalSpy pdf(&api, &ApiClient::pdf_generated);
        api.do_login("demo@example.com", "password", false);
        api.do_signup(QJsonDocument(QJsonObject{}));
        api.save_resource("account", QJsonDocument(QJsonObject{}));
        api.business_selected(QJsonObject{{"_id", "65a000000000000000000001"}});
        api.save_resource("client", QJsonDocument(QJsonObject{{"Name", "Cannot attach to online business"}}));
        api.invoice_details(QJsonDocument(QJsonObject{{"ClientID", "65b000000000000000000001"}}));
        QCOMPARE(login.count(), 1);
        QCOMPARE(signup.count(), 1);
        QCOMPARE(failed.count(), 2);
        QCOMPARE(invoice_error.count(), 1);
        api.save_resource("business", QJsonDocument(QJsonObject{{"Business name", "Demo"}}));
        QSignalSpy businesses(&api, &ApiClient::business_list_received);
        api.get_business_list();
        const auto business_list = qvariant_cast<QJsonDocument>(businesses.last().at(0)).object();
        const auto business = business_list.begin().value().toObject();
        api.business_selected(business);
        api.save_resource("client", QJsonDocument(QJsonObject{{"Name", "Demo client"}}));
        QSignalSpy records(&api, &ApiClient::business_items_received);
        api.get_business_items();
        const auto client = qvariant_cast<QJsonDocument>(records.last().at(0)).object().value("clients").toArray().first().toObject();
        QTemporaryDir output;
        QVERIFY(output.isValid());
        QJsonObject details{{"ClientID", client.value("_id")}, {"invoice_number", "001"},
            {"file_name", "../escape"}, {"file_dir", output.path()}};
        api.invoice_details(QJsonDocument(details));
        QCOMPARE(invoice_error.count(), 2);
        details["file_name"] = "invoice";
        api.invoice_details(QJsonDocument(details));
        api.stock_list(QJsonDocument(QJsonArray{}));
        QCOMPARE(pdf_error.count(), 1);
        // Make the destination unwritable as a file, without changing folder permissions.
        QVERIFY(QDir().mkdir(output.filePath("invoice.pdf")));
        const QJsonDocument stock(QJsonArray{QJsonObject{{"Name", "Demo"}, {"Quantity", 1}, {"Price", 10}}});
        api.stock_list(stock);
        QCOMPARE(pdf_error.count(), 2);
        QVERIFY(pdf.isEmpty());
        QVERIFY(QDir().rmdir(output.filePath("invoice.pdf")));
        api.stock_list(stock);
        QCOMPARE(pdf.count(), 1);
        QSignalSpy stock_saved(&api, &ApiClient::invoice_stock_saved);
        api.save_invoice_stock(QJsonDocument(QJsonObject{{"Name", "Demo stock"}, {"Quantity", 2},
            {"Price", 12.5}, {"StockOnHand", 8}, {"Margin", 0}, {"Unit", "each"}}), 7);
        QCOMPARE(stock_saved.count(), 1);
        QCOMPARE(stock_saved.first().at(1).toULongLong(), quint64(7));
        api.business_selected(QJsonObject{});
        api.stock_list(stock);
        QCOMPARE(pdf_error.count(), 3);
        api.get_stock_list();
        QTest::qWait(100);
        QCOMPARE(connections.count(), 0);
        api.set_offline(false);
        api.business_selected(QJsonObject{{"_id", "65a000000000000000000001"}});
        api.save_resource("client", QJsonDocument(QJsonObject{{"Name", "Online again"}}));
        QTRY_COMPARE(server.saves.size(), 1);
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
        QCOMPARE(title->font().pixelSize(), 30);
        window.dashboard_page()->findChild<QComboBox*>("business_selector")->setCurrentIndex(1);
        QTest::qWait(30);
        window.repaint();
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

    void invoice_terms_save_clear_validate_and_switch_business() {
        ApiClient api(QUrl("http://127.0.0.1:1"), nullptr);
        api.set_offline(true);
        QSignalSpy businesses(&api, &ApiClient::business_list_received);
        api.save_resource("business", QJsonDocument(QJsonObject{{"Business name", "Terms business"}}));
        api.get_business_list();
        auto records = qvariant_cast<QJsonDocument>(businesses.last().first()).object();
        auto business = records.begin().value().toObject();
        const auto id = business.value("_id").toString();
        App::Views::DefaultInvoiceSettings settings;
        settings.set_business(business, true);
        auto* terms = settings.findChild<QPlainTextEdit*>("default_invoice_terms");
        QVERIFY(terms);
        QVERIFY(terms->toPlainText().isEmpty());
        const QString text = "Payment within 14 days.\nPlease quote your invoice number.";
        terms->setPlainText(text);
        connect(&settings, &App::Views::DefaultInvoiceSettings::save_requested, &api, &ApiClient::save_invoice_settings);
        connect(&api, &ApiClient::invoice_settings_saved, &settings, &App::Views::DefaultInvoiceSettings::save_succeeded);
        QSignalSpy saved(&api, &ApiClient::invoice_settings_saved);
        QSignalSpy failed(&api, &ApiClient::invoice_settings_failed);
        auto* save = settings.findChild<QPushButton*>("save_invoice_defaults");
        save->click();
        QCOMPARE(saved.count(), 1);
        business = qvariant_cast<QJsonDocument>(businesses.last().first()).object().value(id).toObject();
        QCOMPARE(business.value("InvoiceTerms").toString(), text);
        settings.set_business(QJsonObject{{"_id", "other-business"}}, true);
        QVERIFY(terms->toPlainText().isEmpty());
        settings.set_business(business, true);
        QCOMPARE(terms->toPlainText(), text);
        settings.resize(850, 700);
        settings.show();
        QTest::qWait(30);
        QVERIFY(settings.grab().save("business-invoice-terms.png"));
        terms->setPlainText(QString(4001, 'x'));
        save->click();
        QCOMPARE(saved.count(), 1);
        QVERIFY(terms->isEnabled());
        QVERIFY(settings.findChild<QLabel*>("invoice_defaults_status")->text().contains("4,000"));
        for (const auto& invalid : {QJsonValue(123), QJsonValue(QString(4001, 'x')), QJsonValue(QJsonValue::Null)})
            api.save_invoice_settings({{"BusinessID", id}, {"InvoicePrefix", "INV-"}, {"InvoiceTerms", invalid}});
        QCOMPARE(failed.count(), 3);
        api.save_invoice_settings({{"BusinessID", id}, {"InvoicePrefix", "JOB-"}});
        business = qvariant_cast<QJsonDocument>(businesses.last().first()).object().value(id).toObject();
        QCOMPARE(business.value("InvoiceTerms").toString(), text);
        terms->clear();
        save->click();
        business = qvariant_cast<QJsonDocument>(businesses.last().first()).object().value(id).toObject();
        QCOMPARE(business.value("InvoiceTerms").toString(), QString{});
        terms->setPlainText(text);
        settings.save_failed("Try again");
        QCOMPARE(terms->toPlainText(), text);
        QVERIFY(terms->isEnabled());
        settings.reset_form();
        QVERIFY(terms->toPlainText().isEmpty());
    }

    void business_invoice_defaults_persist_and_apply() {
        const QJsonObject business{{"_id", "defaults-test-business"}, {"BusinessName", "Example business"}, {"InvoicePrefix", "JOB-"}};
        App::Views::DefaultInvoiceSettings settings;
        settings.set_business(business);
        settings.resize(850, 650); settings.show();
        QVERIFY(settings.grab().save("business-invoice-settings.png"));
        QCOMPARE(settings.invoice_prefix(), "JOB-");
        settings.findChild<QLineEdit*>("default_invoice_file_out_path")->setText(settings_directory_.path());
        settings.findChild<QLineEdit*>("default_invoice_prefix")->clear();
        QSignalSpy requests(&settings, &App::Views::DefaultInvoiceSettings::save_requested);
        settings.findChild<QPushButton*>("save_invoice_defaults")->click();
        QCOMPARE(requests.count(), 1);
        QCOMPARE(requests.first().first().toJsonObject().value("InvoicePrefix").toString(), "");
        settings.save_succeeded();
        settings.reset_form();
        App::Views::DefaultInvoiceSettings reopened;
        reopened.set_business(QJsonObject{{"_id", "defaults-test-business"}, {"InvoicePrefix", ""}});
        QCOMPARE(reopened.invoice_prefix(), "");
        QCOMPARE(reopened.file_out_path(), settings_directory_.path());
        App::Views::InvoiceDetailsInput invoice;
        invoice.prepare_invoice(reopened.invoice_prefix(), reopened.file_out_path());
        QCOMPARE(invoice.findChild<QLineEdit*>("invoice_file_dir")->text(), settings_directory_.path());
        QCOMPARE(invoice.findChild<QLineEdit*>("invoice_prefix")->text(), "");
        QVERIFY(invoice.findChild<QLineEdit*>("invoice_number")->isReadOnly());
        invoice.number_assigned("000001");
        QCOMPARE(invoice.findChild<QLineEdit*>("invoice_file_name")->text(), "000001");
        invoice.prepare_invoice("", reopened.file_out_path());
        invoice.number_assigned("000002");
        QCOMPARE(invoice.findChild<QLineEdit*>("invoice_file_name")->text(), "000002");
        reopened.set_business(QJsonObject{{"_id", "different-business"}});
        QCOMPARE(reopened.invoice_prefix(), "INV-");
        QVERIFY(reopened.file_out_path().isEmpty());
    }

    void forms_save_add_another_failure_and_country_percentage() {
        App::Views::ManagementForm stock("Stock", "stock", {"Name", "Price", "Margin"});
        stock.show();
        auto fields = stock.findChildren<QLineEdit*>("form_input");
        fields[0]->setText("Widget"); fields[1]->setText("10"); fields[2]->setText("25");
        QSignalSpy submissions(&stock, &App::Views::ManagementForm::submit_resource);
        QSignalSpy navigation(&stock, &App::Views::ManagementForm::navigate_to);
        stock.findChild<QPushButton*>("register_button_")->click();
        QCOMPARE(submissions.count(), 1);
        QCOMPARE(qvariant_cast<QJsonDocument>(submissions.first().at(1)).object().value("Margin").toDouble(), 0.25);
        QVERIFY(stock.saving());
        QVERIFY(navigation.isEmpty());
        stock.save_failed("Please retry");
        QCOMPARE(fields[0]->text(), "Widget");
        stock.findChild<QPushButton*>("save_add_new")->click();
        stock.save_succeeded();
        QVERIFY(navigation.isEmpty());
        QVERIFY(fields[0]->text().isEmpty());
        stock.hide(); stock.show();
        for (auto* label : stock.findChildren<QLabel*>()) QVERIFY(!label->text().contains("Saved."));
        fields[0]->setText("Widget 2"); fields[1]->setText("5");
        stock.findChild<QPushButton*>("register_button_")->click();
        stock.save_succeeded();
        QCOMPARE(navigation.count(), 1);
        App::Views::ManagementForm client("Client", "client", {"Name", "Country"});
        auto* country = client.findChild<QComboBox*>("form_country");
        QVERIFY(country); QVERIFY(!country->isEditable()); QVERIFY(country->count() > 150);
        country->setCurrentIndex(country->findData("Australia"));
        client.findChild<QLineEdit*>("form_input")->setText("Customer");
        QSignalSpy data(&client, &App::Views::ManagementForm::submit_resource);
        client.findChild<QPushButton*>("register_button_")->click();
        QCOMPARE(qvariant_cast<QJsonDocument>(data.first().at(1)).object().value("Address").toString(), "Australia");
    }

    void invoice_history_payment_overdue_and_wait_controls() {
        App::Views::InvoicesPage page;
        page.business_selected({{"_id", "business"}});
        QJsonArray invoices;
        for (int index = 0; index < 4; ++index)
            invoices.append(QJsonObject{{"_id", QString::number(index)}, {"InvoiceNumber", QString("INV-%1").arg(index)},
                {"ClientName", "Customer"}, {"DateCreated", QDate::currentDate().addDays(-10).toString(Qt::ISODate)},
                {"DateDue", index == 3 ? "" : QDate::currentDate().addDays(index == 2 ? 0 : -1).toString(Qt::ISODate)},
                {"Paid", index == 1}, {"Total", 25}});
        page.populate_invoices(QJsonDocument(QJsonObject{{"BusinessID", "business"}, {"invoices", invoices}, {"offset", 0}, {"has_more", false}}));
        auto* table = page.findChild<QTableWidget*>("invoice_records");
        auto* payment = page.findChild<QPushButton*>("invoice_payment");
        auto* export_pdf = page.findChild<QPushButton*>("invoice_export");
        QVERIFY(table && payment && export_pdf);
        QVERIFY(!payment->isEnabled());
        QVERIFY(!export_pdf->isEnabled());
        QCOMPARE(table->rowCount(), 4);
        QCOMPARE(table->item(0, 5)->text(), "Unpaid - OVERDUE");
        QCOMPARE(table->item(1, 5)->text(), "Paid");
        QCOMPARE(table->item(2, 5)->text(), "Unpaid"); // Due today is not overdue.
        QCOMPARE(table->item(3, 5)->text(), "Unpaid"); // Missing original date is unknown.
        table->selectRow(0);
        QVERIFY(page.findChild<QLabel*>("invoice_selection")->text().contains("Selected invoice: INV-0"));
        QSignalSpy changes(&page, &App::Views::InvoicesPage::payment_requested);
        payment->click();
        QCOMPARE(changes.count(), 1);
        QCOMPARE(changes.first().at(0).toString(), "0");
        QCOMPARE(changes.first().at(1).toBool(), true);
        QVERIFY(!payment->isEnabled());
        page.payment_updated("0", true);
        QCOMPARE(table->item(0, 5)->text(), "Paid");
        QCOMPARE(payment->text(), "Mark as unpaid");
        page.payment_updated("0", false);
        QCOMPARE(table->item(0, 5)->text(), "Unpaid - OVERDUE");
        QVERIFY(export_pdf->isEnabled());
        page.export_wait(1);
        QVERIFY(!export_pdf->isEnabled());
        QTRY_VERIFY_WITH_TIMEOUT(export_pdf->isEnabled(), 2500);
        if (const auto preview = qEnvironmentVariable("INVOICE_PDF_PREVIEW_DIR"); !preview.isEmpty()) {
            page.resize(1100, 750); page.show(); QTest::qWait(50);
            page.grab().save(preview + "/invoice-history.png");
        }
        page.business_selected({{"_id", "another"}});
        QCOMPARE(table->rowCount(), 0);
        QVERIFY(!payment->isEnabled());
    }

    void invoice_creation_paid_checkbox_is_submitted_and_reset() {
        App::Views::InvoiceDetailsInput page;
        const auto business = business_list().object().begin().value().toObject();
        page.business_selected(business);
        page.populate_clients(business_items(business.value("_id").toString()));
        page.findChild<QComboBox*>("invoice_client")->setCurrentIndex(1);
        auto* paid = page.findChild<QCheckBox*>("invoice_paid");
        QVERIFY(paid);
        QVERIFY(!paid->isChecked());
        paid->setChecked(true);
        QSignalSpy submitted(&page, &App::Views::InvoiceDetailsInput::set_invoice_details);
        page.findChild<QPushButton*>("invoice_next")->click();
        QCOMPARE(submitted.count(), 1);
        QVERIFY(qvariant_cast<QJsonDocument>(submitted.first().first()).object().value("paid").toBool());
        page.reset_form();
        QVERIFY(!paid->isChecked());
    }

    void offline_archive_preserves_details_and_stock() {
        Application::Invoices::OfflineInvoiceSession session;
        const auto business = session.save("business", {{"Business name", "Original business"}, {"PaymentTerms", "14"}}, {});
        const auto client = session.save("client", {{"Name", "Original client"}}, business);
        const auto stock = session.save("stock", {{"Name", "Widget"}, {"Quantity", 5}, {"Price", 10}}, business);
        QTemporaryDir output;
        QVERIFY(output.isValid());
        session.begin_invoice({{"ClientID", client}, {"file_dir", output.path()},
            {"date_created", "2026-01-01"}, {"date_due", "2026-01-15"}, {"paid", true}}, business);
        const QJsonArray lines{QJsonObject{{"_id", stock}, {"Name", "Widget"}, {"Quantity", 2}, {"Price", 10}}};
        session.generate_pdf(lines);
        session.generate_pdf(lines);
        QCOMPARE(session.invoices.size(), 1);
        const auto id = session.invoices.first().toObject().value("_id").toString();
        session.clear_invoice();
        QCOMPARE(session.invoice_list(business).value("invoices").toArray().size(), 1);
        QVERIFY(session.invoice_list("another").value("invoices").toArray().isEmpty());
        session.clients = {}; // An archived invoice is independent of current contacts.
        session.set_invoice_paid(business, id, false);
        const auto record = session.saved_invoice(business, id);
        QVERIFY(!record.value("Paid").toBool());
        QCOMPARE(record.value("Client").toObject().value("Name").toString(), "Original client");
        QCOMPARE(record.value("DateDue").toString(), "2026-01-15");
        QVERIFY(Infrastructure::PDF::InvoicePdfGenerator::render_record(record).startsWith("%PDF-"));
        QCOMPARE(session.stocks.first().toObject().value("Quantity").toInt(), 3);
        QVERIFY_EXCEPTION_THROWN(session.saved_invoice("another", id), std::invalid_argument);
    }

    void offline_numbers_stock_decrement_and_retry() {
        Application::Invoices::OfflineInvoiceSession session;
        const auto business = session.save("business", {{"Business name", "A"}}, {});
        const auto client = session.save("client", {{"Name", "Customer"}}, business);
        const auto stock = session.save("stock", {{"Name", "Widget"}, {"Quantity", 5}, {"Price", 10}}, business);
        QTemporaryDir output;
        QJsonObject details{{"ClientID", client}, {"file_dir", output.path()}, {"invoice_prefix", "JOB-"}};
        session.begin_invoice(details, business);
        QCOMPARE(session.invoice_number(), "JOB-000001");
        const QJsonArray lines{QJsonObject{{"_id", stock}, {"Name", "Widget"}, {"Quantity", 2}, {"Price", 10}}};
        QVERIFY(QFile::exists(session.generate_pdf(lines)));
        QCOMPARE(session.stocks.first().toObject().value("Quantity").toInt(), 3);
        session.generate_pdf(lines);
        QCOMPARE(session.stocks.first().toObject().value("Quantity").toInt(), 3);
        details["invoice_prefix"] = "";
        session.begin_invoice(details, business);
        QCOMPARE(session.invoice_number(), "000002");
        const QJsonArray too_many{QJsonObject{{"_id", stock}, {"Name", "Widget"}, {"Quantity", 4}, {"Price", 10}}};
        QVERIFY_EXCEPTION_THROWN(session.generate_pdf(too_many), std::runtime_error);
        QCOMPARE(session.stocks.first().toObject().value("Quantity").toInt(), 3);
    }

    void offline_stock_shortage_choices_data() {
        QTest::addColumn<QString>("policy");
        QTest::addColumn<int>("balance");
        QTest::newRow("zero") << QString("zero") << 0;
        QTest::newRow("negative") << QString("negative") << -3;
    }

    void offline_stock_shortage_choices() {
        QFETCH(QString, policy);
        QFETCH(int, balance);
        Application::Invoices::OfflineInvoiceSession session;
        const auto business = session.save("business", {{"Business name", "A"}}, {});
        const auto client = session.save("client", {{"Name", "Customer"}}, business);
        const auto stock = session.save("stock", {{"Name", "Widget"}, {"Quantity", 5}, {"Price", 10}}, business);
        const auto other = session.save("stock", {{"Name", "Other"}, {"Quantity", 10}, {"Price", 1}}, business);
        QTemporaryDir output;
        QVERIFY(output.isValid());
        const QJsonObject details{{"ClientID", client}, {"file_dir", output.path()}};
        session.begin_invoice(details, business);
        const auto before = session.stocks;
        const QJsonObject line{{"_id", stock}, {"Name", "Widget"}, {"Quantity", 4}, {"Price", 10}};
        const QJsonArray lines{line, line, QJsonObject{{"_id", other}, {"Name", "Other"}, {"Quantity", 2}, {"Price", 1}}};
        bool rejected = false;
        try {
            session.generate_pdf(lines);
        } catch (const Domain::Stock::StockShortage& error) {
            rejected = true;
            QCOMPARE(error.items.size(), 1);
            const auto item = error.items.first().toObject();
            QCOMPARE(item.value("_id").toString(), stock);
            QCOMPARE(item.value("Available").toInt(), 5);
            QCOMPARE(item.value("Requested").toInt(), 8);
        }
        QVERIFY(rejected);
        QCOMPARE(session.stocks, before);
        QVERIFY(QDir(output.path()).entryList({"*.pdf"}, QDir::Files).isEmpty());
        QVERIFY_EXCEPTION_THROWN(session.generate_pdf(lines, "invalid"), std::invalid_argument);
        QCOMPARE(session.stocks, before);
        const auto path = session.generate_pdf(lines, policy);
        QVERIFY(QFile::exists(path));
        QCOMPARE(session.stocks.at(0).toObject().value("Quantity").toInt(), balance);
        QCOMPARE(session.stocks.at(1).toObject().value("Quantity").toInt(), 8);
        const auto completed = session.stocks;
        session.generate_pdf(lines, policy);
        session.generate_pdf(lines); // A retry does not need to reconfirm completed stock changes.
        QCOMPARE(session.stocks, completed);
        QVERIFY_EXCEPTION_THROWN(session.generate_pdf(QJsonArray{line}, policy), std::runtime_error);
        QCOMPARE(session.stocks, completed);
        session.begin_invoice(details, business);
        QVERIFY_EXCEPTION_THROWN(session.generate_pdf(QJsonArray{line}), Domain::Stock::StockShortage);
        QCOMPARE(session.stocks, completed);
        session.generate_pdf(QJsonArray{line}, "negative");
        QCOMPARE(session.stocks.at(0).toObject().value("Quantity").toInt(), balance - 4);
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
        QCOMPARE(chosen.last().at(0).toJsonObject().value("_id").toString(), "65a000000000000000000002");
        QVERIFY(button(dashboard, "Create stock item")->isEnabled());
        QVERIFY(button(dashboard, "Create new client")->isEnabled());
        QVERIFY(button(dashboard, "Create new invoice")->isEnabled());

        dashboard.populate_business_list(business_list());
        QCOMPARE(selector->currentData().toJsonObject().value("_id").toString(), "65a000000000000000000002");
        dashboard.resize(720, 420);
        dashboard.show();
        QCoreApplication::processEvents();
        QVERIFY(dashboard.grab().save("dashboard-business-selector.png"));

        dashboard.populate_business_list(QJsonDocument(QJsonObject{}));
        QVERIFY(!dashboard.has_business());
        QVERIFY(chosen.last().at(0).toJsonObject().isEmpty());
        QVERIFY(!button(dashboard, "Create new invoice")->isEnabled());
    }

    void controls_have_consistent_roles_and_geometry_data() {
        QTest::addColumn<QSize>("window_size");
        QTest::newRow("standard") << QSize(800, 800);
        QTest::newRow("compact") << QSize(640, 640);
        QTest::newRow("desktop") << QSize(1080, 860);
    }

    void controls_have_consistent_roles_and_geometry() {
        QFETCH(QSize, window_size);
        TestAccountManager account;
        App::Views::MainWindow window(account);
        window.resize(window_size);
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
                if (auto* scroll = page->findChild<QScrollArea*>(); scroll && scroll->isAncestorOf(button)) {
                    scroll->ensureWidgetVisible(button);
                    QCoreApplication::processEvents();
                    const QRect visible_bounds(button->mapTo(scroll->viewport(), QPoint()), button->size());
                    QVERIFY2(scroll->viewport()->rect().contains(visible_bounds),
                             qPrintable(name + ": unreachable " + button->text()));
                }
                const QRect bounds(button->mapTo(page, QPoint()), button->size());
                QVERIFY2(page->rect().contains(bounds), qPrintable(name + ": clipped " + button->text()));
            }
            if (auto* scroll = page->findChild<QScrollArea*>()) {
                scroll->verticalScrollBar()->setValue(0);
                scroll->horizontalScrollBar()->setValue(0);
            }
            QTest::qWait(30);
            window.repaint();
            QVERIFY(window.grab().save(name + "-" + QTest::currentDataTag() + "-ui.png"));
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
        QCOMPARE(server.saves.last().value("BusinessID").toString(), "65a000000000000000000001");
        selector->setCurrentIndex(2);
        api.save_resource("client", payload);
        QTRY_COMPARE(saved.count(), 2);
        QCOMPARE(server.saves.last().value("BusinessID").toString(), "65a000000000000000000002");
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
        int completed = 0;
        for (const auto& id : {QString("65a000000000000000000001"), QString("65a000000000000000000002")}) {
            api.business_selected(QJsonObject{{"_id", id}});
            api.invoice_details(QJsonDocument(QJsonObject{{"BusinessID", "STALE"}, {"ClientID", "65b000000000000000000001"}}));
            QTRY_VERIFY(!server.invoices.isEmpty() && server.invoices.last().value("BusinessID").toString() == id);
            api.get_stock_list();
            QTRY_VERIFY(!server.stock_businesses.isEmpty() && server.stock_businesses.last() == id);
            // Receipt by the server does not mean the client has handled the reply.
            // Wait before switching businesses, which deliberately discards stale replies.
            ++completed;
            QTRY_COMPARE(started.count(), completed);
            QTRY_COMPARE(stock.count(), completed);
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
        api.business_selected(QJsonObject{{"_id", "65a000000000000000000001"}});
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
        QTRY_COMPARE(clients->count(), 2);
        QCOMPARE(server.items_businesses.size(), 2); // Refresh inventory and business settings for each new invoice.
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
        QCOMPARE(server.invoices.first().value("ClientID").toString(), "65b000000000000000000001");
        QCOMPARE(server.invoices.first().value("BusinessID").toString(), "65a000000000000000000001");
        QVERIFY(!server.invoices.first().contains("website"));
        QVERIFY(!page->findChild<QLineEdit*>("website"));
        QTRY_COMPARE(window.findChild<QStackedWidget*>()->currentWidget(), window.new_invoice_stock_page());
        controller.page_navigation(Page::Dashboard);
        businesses->setCurrentIndex(2);
        QCOMPARE(clients->count(), 1);
        QVERIFY(!next->isEnabled());
        QVERIFY(page->findChild<QLabel*>("invoice_client_details")->text().isEmpty());
        page->populate_clients(business_items("65a000000000000000000001"));
        QCOMPARE(clients->count(), 1);
        controller.page_navigation(Page::NewInvoice);
        QTRY_COMPARE(clients->count(), 2);
        QCOMPARE(server.items_businesses.last(), "65a000000000000000000002");
        clients->setCurrentIndex(1);
        emit window.logged_out();
        QVERIFY(!next->isEnabled());
        QCOMPARE(clients->count(), 1);
    }

    void invoice_clients_empty_error_and_refresh_states() {
        App::Views::InvoiceDetailsInput page;
        page.business_selected(QJsonObject{{"_id", "65a000000000000000000001"}});
        auto* clients = page.findChild<QComboBox*>("invoice_client");
        auto* next = page.findChild<QPushButton*>("invoice_next");
        auto* refresh = page.findChild<QPushButton*>("refresh_invoice_clients");
        auto empty = business_items("65a000000000000000000001").object();
        empty["clients"] = QJsonArray{};
        page.populate_clients(QJsonDocument(empty));
        QVERIFY(!clients->isEnabled());
        QVERIFY(!next->isEnabled());
        QVERIFY(page.findChild<QLabel*>("invoice_client_status")->text().contains("No clients saved"));
        page.populate_clients(business_items("65a000000000000000000001"));
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
        api.business_selected(QJsonObject{{"_id", "65a000000000000000000001"}});
        QSignalSpy failed(&api, &ApiClient::invoice_failed);
        api.invoice_details(QJsonDocument(QJsonObject{}));
        QCOMPARE(failed.count(), 1);
        QVERIFY(server.invoices.isEmpty());
    }

    void new_invoice_stock_can_be_temporary_or_saved() {
        RecordingServer server;
        QVERIFY(server.start());
        ApiClient api(server.url(), nullptr);
        api.business_selected(QJsonObject{{"_id", "65a000000000000000000001"}});
        App::Views::NewInvoiceStock page;
        page.business_selected(QJsonObject{{"_id", "65a000000000000000000001"}});
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
        QCOMPARE(server.saves.first().value("Unit").toString(), "each");
        QCOMPARE(server.saves.first().value("BusinessID").toString(), "65a000000000000000000001");
        QCOMPARE(page.stock_items.last().toObject().value("Quantity").toInt(), 3);
        QCOMPARE(page.stock_items.last().toObject().value("_id").toString(), "65c000000000000000000002");

        page.findChild<QComboBox*>("invoice_item_source")->setCurrentIndex(1);
        quantity->setText("1");
        add->click(); // Uses the saved price when no override is entered.
        QCOMPARE(page.stock_items.size(), 3);
        QCOMPARE(page.stock_items.last().toObject().value("Price").toDouble(), 12.5);
        QCOMPARE(server.saves.size(), 1);
    }

    void invoice_lines_can_be_removed_data() {
        QTest::addColumn<bool>("offline");
        QTest::newRow("account") << false;
        QTest::newRow("offline") << true;
    }

    void invoice_lines_can_be_removed() {
        QFETCH(bool, offline);
        App::Views::NewInvoiceStock page;
        page.set_offline(offline);
        const QJsonObject saved{{"_id", "65c000000000000000000001"}, {"Name", "Copper pipe"},
                                {"Quantity", 10}, {"Price", 12.5}};
        page.populate_stock_list(QJsonDocument(QJsonArray{saved}));
        page.findChild<QComboBox*>("invoice_item_source")->setCurrentIndex(1);
        auto* quantity = qobject_cast<QLineEdit*>(page.invoice_body_form_fields.value("quantity"));
        auto* add = page.findChild<QPushButton*>("invoice_add_item");
        auto* finish = page.findChild<QPushButton*>("invoice_finish");
        auto* selector = page.findChild<QComboBox*>("invoice_saved_stock");
        QSignalSpy submitted(&page, &App::Views::NewInvoiceStock::add_item_list_to_invoice);
        QSignalSpy stock_saves(&page, &App::Views::NewInvoiceStock::save_invoice_stock);
        for (int count : {1, 2, 3}) {
            quantity->setText(QString::number(count));
            add->click();
        }
        const auto buttons = page.findChildren<QPushButton*>("invoice_remove_item");
        QCOMPARE(buttons.size(), 3);
        // Duplicate stock lines must be removed by row, not by stock ID or name.
        buttons[1]->click();
        QCOMPARE(page.stock_items.size(), 2);
        QCOMPARE(page.stock_items[0].toObject().value("Quantity").toInt(), 1);
        QCOMPARE(page.stock_items[1].toObject().value("Quantity").toInt(), 3);
        buttons[0]->click();
        QCOMPARE(page.stock_items.size(), 1);
        QCOMPARE(page.stock_items[0].toObject().value("Quantity").toInt(), 3);
        QCOMPARE(selector->count(), 1);
        QCOMPARE(selector->currentData().toJsonObject(), saved);
        QVERIFY(stock_saves.isEmpty());

        finish->click();
        QCOMPARE(submitted.count(), 1);
        QCOMPARE(qvariant_cast<QJsonDocument>(submitted[0][0]).array(), page.stock_items);
        QVERIFY(!buttons[2]->isEnabled());
        buttons[2]->click();
        QCOMPARE(page.stock_items.size(), 1);
        page.pdf_failed("Try again");
        QVERIFY(buttons[2]->isEnabled());
        buttons[2]->click();
        QVERIFY(page.stock_items.isEmpty());
        finish->click();
        QCOMPARE(submitted.count(), 1); // An empty invoice cannot be submitted.
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        QVERIFY(page.findChildren<QPushButton*>("invoice_remove_item").isEmpty());

        quantity->setText("1");
        add->click();
        auto* remove = page.findChild<QPushButton*>("invoice_remove_item");
        QVERIFY(remove);
        finish->click();
        page.pdf_generated("invoice.pdf");
        QVERIFY(!remove->isEnabled());
        finish->click();
        page.pdf_failed("Could not save another copy");
        QVERIFY(!remove->isEnabled()); // Finalized invoices remain immutable on retry failure.
        page.reset_invoice();
        QVERIFY(page.stock_items.isEmpty());
        QVERIFY(page.findChildren<QPushButton*>("invoice_remove_item").isEmpty());
        QVERIFY(add->isEnabled());
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
        api.business_selected(QJsonObject{{"_id", "65a000000000000000000001"}});
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

    void optional_record_fields_data() {
        QTest::addColumn<bool>("offline");
        QTest::newRow("online") << false;
        QTest::newRow("offline") << true;
    }

    void optional_record_fields() {
        QFETCH(bool, offline);
        RecordingServer server;
        QVERIFY(server.start());
        QSignalSpy connections(server.tcp, &QTcpServer::newConnection);
        TestAccountManager account;
        ApiClient api(server.url(), &account);
        App::Views::MainWindow window(account);
        Application::Controllers::AppController controller(&window, account, &api);
        if (offline)
            controller.start_offline();
        const auto input = [](App::Views::ManagementForm* form, const QString& field) -> QLineEdit* {
            for (auto* edit : form->findChildren<QLineEdit*>("form_input"))
                if (edit->placeholderText() == field) return edit;
            return nullptr;
        };
        const auto submit = [](App::Views::ManagementForm* form) {
            form->findChild<QPushButton*>("register_button_")->click();
        };
        QSignalSpy saved(&api, &ApiClient::resource_saved);
        auto* business = window.business_settings_page();
        QSignalSpy business_data(business, &App::Views::ManagementForm::submit_resource);
        input(business, "Business name")->setText("   ");
        submit(business);
        QVERIFY(business_data.isEmpty());
        input(business, "Business name")->setText("Minimal business");
        input(business, "Website")->setText("file:///invalid-website");
        submit(business);
        QVERIFY(business_data.isEmpty());
        input(business, "Website")->clear();
        submit(business);
        QTRY_COMPARE(saved.count(), 1);
        const auto business_payload = qvariant_cast<QJsonDocument>(business_data.first().at(1)).object();
        for (const auto& field : {"ABN", "ACN", "Business phone", "Website", "Address", "LogoData"})
            QVERIFY(business_payload.value(field).toString().isEmpty());
        if (!offline)
            api.business_selected(QJsonObject{{"_id", "65a000000000000000000001"}});

        auto* client = window.client_page();
        QSignalSpy client_data(client, &App::Views::ManagementForm::submit_resource);
        submit(client);
        QVERIFY(client_data.isEmpty());
        input(client, "Name")->setText("Minimal client");
        submit(client);
        QTRY_COMPARE(saved.count(), 2);
        const auto client_payload = qvariant_cast<QJsonDocument>(client_data.first().at(1)).object();
        QVERIFY(client_payload.value("Address").toString().isEmpty());
        QVERIFY(client_payload.value("Email").toString().isEmpty());
        QVERIFY(client_payload.value("Phone").toString().isEmpty());
        input(client, "Name")->setText("Partial address client");
        input(client, "City")->setText("Sydney");
        input(client, "Postcode")->setText("2000");
        submit(client);
        QTRY_COMPARE(saved.count(), 3);
        QCOMPARE(qvariant_cast<QJsonDocument>(client_data.last().at(1)).object().value("Address").toString(), "Sydney, 2000");

        auto* stock = window.stock_settings_page();
        QSignalSpy stock_data(stock, &App::Views::ManagementForm::submit_resource);
        input(stock, "Name")->setText("Minimal stock");
        submit(stock);
        QVERIFY(stock_data.isEmpty()); // Price is still required.
        input(stock, "Price")->setText("12.50");
        input(stock, "Quantity")->setText("1.5");
        submit(stock);
        QVERIFY(stock_data.isEmpty()); // Optional values must be valid when entered.
        input(stock, "Quantity")->clear();
        input(stock, "Margin")->setText("invalid");
        submit(stock);
        QVERIFY(stock_data.isEmpty());
        input(stock, "Margin")->clear();
        submit(stock);
        QTRY_COMPARE(saved.count(), 4);
        const auto stock_payload = qvariant_cast<QJsonDocument>(stock_data.first().at(1)).object();
        QVERIFY(stock_payload.value("Price").isDouble());
        QCOMPARE(stock_payload.value("Price").toDouble(), 12.5);
        QCOMPARE(stock_payload.value("Quantity").toInt(-1), 0);
        QCOMPARE(stock_payload.value("Margin").toDouble(-1), 0.0);
        QCOMPARE(stock_payload.value("Unit").toString(), "each");
        QVERIFY(stock_payload.value("Keywords").toString().isEmpty());
        if (offline) {
            QSignalSpy records(&api, &ApiClient::business_items_received);
            api.get_business_items();
            const auto items = qvariant_cast<QJsonDocument>(records.last().at(0)).object();
            QCOMPARE(items.value("stocks").toArray().first().toObject().value("Price").toDouble(), 12.5);
            QTemporaryDir output;
            QVERIFY(output.isValid());
            QSignalSpy pdf(&api, &ApiClient::pdf_generated);
            api.invoice_details(QJsonDocument(QJsonObject{
                {"ClientID", items.value("clients").toArray().first().toObject().value("_id")},
                {"invoice_number", "MINIMAL-001"}, {"file_name", "minimal"}, {"file_dir", output.path()}}));
            api.stock_list(QJsonDocument(QJsonArray{QJsonObject{{"Name", "Minimal stock"}, {"Quantity", 1}, {"Price", 12.5}}}));
            QCOMPARE(pdf.count(), 1);
            QFile file(output.filePath("minimal.pdf"));
            QVERIFY(file.open(QIODevice::ReadOnly));
            QCOMPARE(file.read(5), QByteArray("%PDF-"));
            QTest::qWait(50);
            QCOMPARE(connections.count(), 0);
        } else {
            QCOMPARE(server.saves.size(), 4);
            QCOMPARE(server.saves.last().value("Price").toDouble(), 12.5);
            QCOMPARE(server.saves.last().value("Quantity").toInt(-1), 0);
            QCOMPARE(server.saves.last().value("Unit").toString(), "each");
            QVERIFY(server.saves.first().value("ABN").toString().isEmpty());
        }
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
        QCOMPARE(form->findChild<QLineEdit*>("business_logo_path")->text(), path);
        form->save_succeeded();
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
        QCOMPARE(stock->columnCount(), 6);
        QCOMPARE(stock->item(0, 0)->data(Qt::UserRole).toString(), "65c000000000000000000001");
        for (int column = 0; column < stock->columnCount(); ++column)
            QVERIFY(!stock->item(0, column)->text().contains("65c000"));
        QCOMPARE(stock->item(0, 1)->data(Qt::DisplayRole).toInt(), 10);
        QCOMPARE(stock->item(0, 5)->text(), "pipe, copper");
        QCOMPARE(stock->editTriggers(), QAbstractItemView::NoEditTriggers);
        auto* tabs = page->findChild<QTabWidget*>();
        QCOMPARE(tabs->count(), 3);
        tabs->setCurrentIndex(1);
        tabs->setCurrentIndex(2);
        QApplication::clipboard()->clear();
        QTimer::singleShot(0, stock, [stock]() {
            auto* menu = stock->findChild<QMenu*>();
            if (menu) QTest::mouseClick(menu, Qt::LeftButton, Qt::NoModifier,
                                       menu->actionGeometry(menu->actions().last()).center());
        });
        emit stock->customContextMenuRequested(stock->visualItemRect(stock->item(0, 0)).center());
        QCOMPARE(QApplication::clipboard()->text(), "65c000000000000000000001");
        QCOMPARE(server.items_businesses.size(), 1);
        QVERIFY(window.grab().save("business-records-stock.png"));
        controller.page_navigation(Page::Dashboard);
        browse->click();
        QCOMPARE(server.items_businesses.size(), 1);
        page->findChild<QPushButton*>("refresh_items")->click();
        QTRY_COMPARE(server.items_businesses.size(), 2);
        QTRY_VERIFY(page->findChild<QPushButton*>("refresh_items")->isEnabled());
        auto empty = business_items("65a000000000000000000001").object();
        empty["clients"] = QJsonArray{};
        empty["stocks"] = QJsonArray{};
        page->populate_items(QJsonDocument(empty));
        QCOMPARE(stock->rowCount(), 0);
        QCOMPARE(tabs->tabText(2), "Stock (0)");
        page->populate_items(business_items("65a000000000000000000002"));
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
        api.business_selected(QJsonObject{{"_id", "65a000000000000000000001"}});
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
        api.business_selected(QJsonObject{{"_id", "65a000000000000000000002"}});
        api.get_business_items();
        QTRY_COMPARE(received.count(), 7);
        QCOMPARE(server.items_businesses.last(), "65a000000000000000000002");
        QCOMPARE(qvariant_cast<QJsonDocument>(received.last().at(0)).object().value("BusinessID").toString(), "65a000000000000000000002");
        api.clear_session();
        api.business_selected(QJsonObject{{"_id", "65a000000000000000000002"}});
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
        api.business_selected(QJsonObject{{"_id", "65a000000000000000000001"}});
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
        api.business_selected(QJsonObject{{"_id", "65a000000000000000000001"}});
        api.get_business_items();
        api.business_selected(QJsonObject{{"_id", "65a000000000000000000002"}});
        api.get_business_items();
        api.clear_session();
        api.business_selected(QJsonObject{{"_id", "65a000000000000000000001"}});
        api.get_business_items();
        QTRY_COMPARE(received.count(), 1);
        QCOMPARE(qvariant_cast<QJsonDocument>(received.at(0).at(0)).object().value("BusinessID").toString(), "65a000000000000000000001");
        QVERIFY(failed.isEmpty());
    }
};

QTEST_MAIN(BusinessSessionTests)
#include "BusinessSessionTests.moc"
