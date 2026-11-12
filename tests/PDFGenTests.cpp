#include <QtTest/QtTest>
#include "Domain/Invoices/Invoice.h"
#include "Infrastructure/Pdf/InvoicePdfGenerator.h"
#include <cstdlib>
#include <QTemporaryDir>
#include <vector>
namespace fs = std::filesystem;
class TestPDFGen : public QObject {
    Q_OBJECT
  private slots:
    void initTestCase() { QVERIFY(output_.isValid()); }
    void pdf_creation_test();
    void pdf_multiple_page_creation_test();
    void pdf_three_long_lines();
    void pdf_client_details();

  private:
    QTemporaryDir output_;
    const std::string pdf_out_path = output_.filePath("test_invoice").toStdString();
    const std::string pdf_out_path_multi = output_.filePath("test_invoice_multi").toStdString();
    const std::string pdf_out_path_long = output_.filePath("test_invoice_long").toStdString();
};

void TestPDFGen::pdf_client_details() {
    auto invoice = std::make_shared<Invoice>();
    auto business = std::make_shared<BusinessRepository>();
    business->setBizName("Example Plumbing");
    business->set_website_url("https://example.com");
    business->set_business_logo_path("missing-original-logo.png");
    QFile logo(QFileInfo(QString::fromUtf8(__FILE__)).absolutePath() + "/test_resources/logo.png");
    QVERIFY(logo.open(QIODevice::ReadOnly));
    business->set_business_logo_data(logo.readAll().toBase64().toStdString());
    QCOMPARE(business->get_business_logo_path(), std::string("missing-original-logo.png"));
    invoice->setBusiness(business);
    auto client = std::make_shared<Client>();
    client->setClientID("CLI1");
    client->setName("Example Client");
    client->setAddress("Suite 12, Example Commercial Centre, 100 Example Street, Sydney NSW 2000, Australia\nAccounts Department");
    client->setEmail("accounts@example.com");
    client->setPhoneNumber("02 1234 5678");
    invoice->setClient(client);
    for (int index = 0; index < 60; ++index) {
        auto item = std::make_shared<StockItem>();
        item->setStockID("STK" + std::to_string(index));
        item->set_description("Copper pipe");
        item->setStdPrice(12.5);
        invoice->addStockItem(item, 2);
    }
    const auto path = output_.filePath("invoice-client-test.pdf");
    invoice->set_file_name(path.toStdString());
    Infrastructure::PDF::InvoicePdfGenerator generator(invoice);
    QVERIFY(generator.peece_template());
    QVERIFY(QFileInfo(path).size() > 0);
}

void TestPDFGen::pdf_multiple_page_creation_test() {
    Invoice test_invoice;
    std::shared_ptr<BusinessRepository> biz = std::make_shared<BusinessRepository>();
    biz->set_website_url("https://google.com");
    biz->setName("My Business");
    QString logoPath = QFileInfo(QString::fromUtf8(__FILE__)).absolutePath() + "/test_resources/logo.png";
    std::cout << logoPath.toStdString() << std::endl;
    biz->set_biz_logo_url(logoPath.toStdString());
    test_invoice.setBusiness(biz);
    const int items_to_generate = 75;
    std::vector<StockItem> items;
    items.reserve(items_to_generate);
    for (int i = 0; i < items_to_generate; ++i) {
        StockItem item;
        const int rand_max_value = 100;
        int randomNum = rand() % 101;
        float random_num = static_cast<float>(rand()) / static_cast<float>(rand_max_value);
        item.set_description("AAAAXXXX");
        item.setStdPrice(random_num);
        item.setStockOnHand(100);
        items.push_back(item);
        test_invoice.addStockItem(std::make_shared<StockItem>(item), 2);
    }

    const std::string pdf_num = "0";
    test_invoice.setInvoiceID("INV-1001");
    test_invoice.set_file_name(pdf_out_path_multi + pdf_num + ".pdf");
    test_invoice.setCurrentDate("2024-10-01");
    test_invoice.setDueDate("2024-10-15");
    test_invoice.setTotalAmount(500.0f);
    test_invoice.set_website("www.google.com");

    Infrastructure::PDF::InvoicePdfGenerator pdf_gen(std::make_shared<Invoice>(test_invoice));

    pdf_gen.peece_template();

    QVERIFY(fs::exists(pdf_out_path_long + pdf_num + ".pdf"));
}
void TestPDFGen::pdf_three_long_lines() {
    // TO DO cut off the line at 100 characters. But make the line cell longer vertically until then.
    Invoice test_invoice;
    std::shared_ptr<BusinessRepository> biz = std::make_shared<BusinessRepository>();
    biz->set_website_url("https://google.com");
    biz->setName("My Business");
    QString logoPath = QFileInfo(QString::fromUtf8(__FILE__)).absolutePath() + "/test_resources/logo.png";
    std::cout << logoPath.toStdString() << std::endl;
    biz->set_biz_logo_url(logoPath.toStdString());
    test_invoice.setBusiness(biz);
    const int items_to_generate = 3;
    std::vector<StockItem> items;
    items.reserve(items_to_generate);
    for (int i = 0; i < items_to_generate; ++i) {
        StockItem item;
        const int rand_max_value = 100;
        int randomNum = rand() % 101;
        float random_num = static_cast<float>(rand()) / static_cast<float>(rand_max_value);
        item.setStockID("1234a");
        item.set_description(
            "The quick brown fox jumped over the something dog and into the lake because he was scared of the "
            "cat that looked as if it was going to attack");
        item.setStdPrice(random_num);
        item.setStockOnHand(100);
        items.push_back(item);
        test_invoice.addStockItem(std::make_shared<StockItem>(item), 2);
    }

    const std::string pdf_num = "0";
    test_invoice.setInvoiceID("INV-1001");
    test_invoice.set_file_name(pdf_out_path_long + pdf_num + ".pdf");
    test_invoice.setCurrentDate("2024-10-01");
    test_invoice.setDueDate("2024-10-15");
    test_invoice.setTotalAmount(500.0f);
    test_invoice.set_website("www.google.com");

    Infrastructure::PDF::InvoicePdfGenerator pdf_gen(std::make_shared<Invoice>(test_invoice));

    pdf_gen.peece_template();

    QVERIFY(fs::exists(pdf_out_path_multi + pdf_num + ".pdf"));
}

void TestPDFGen::pdf_creation_test() {

    Invoice test_invoice;
    std::shared_ptr<BusinessRepository> biz = std::make_shared<BusinessRepository>();
    biz->set_website_url("http://google.com");
    biz->setName("My Business");
    QString logoPath = QFileInfo(QString::fromUtf8(__FILE__)).absolutePath() + "/test_resources/logo.png";
    std::cout << logoPath.toStdString() << std::endl;
    biz->set_biz_logo_url(logoPath.toStdString());
    test_invoice.setBusiness(biz);

    std::vector<StockItem> items{};

    StockItem item1;
    StockItem item2;
    StockItem item3;
    StockItem item4;
    StockItem item5;

    item1.set_description("Item One");
    item1.setStdPrice(10.0f);
    item1.setStockOnHand(100);

    item2.set_description("Item Two");
    item2.setStdPrice(20.0f);
    item2.setStockOnHand(200);

    item3.set_description("Item Three");
    item3.setStdPrice(30.0f);
    item3.setStockOnHand(300);

    item4.set_description("Item Four");
    item4.setStdPrice(40.0f);
    item4.setStockOnHand(400);

    item5.set_description("Item Five");
    item5.setStdPrice(50.0f);
    item5.setStockOnHand(500);

    items.push_back(item1);
    items.push_back(item2);
    items.push_back(item3);
    items.push_back(item4);
    items.push_back(item5);

    test_invoice.addStockItem(std::make_shared<StockItem>(items[0]), 2);
    test_invoice.addStockItem(std::make_shared<StockItem>(items[1]), 3);
    test_invoice.addStockItem(std::make_shared<StockItem>(items[2]), 1);
    test_invoice.addStockItem(std::make_shared<StockItem>(items[3]), 5);
    test_invoice.addStockItem(std::make_shared<StockItem>(items[4]), 4);
    const std::string pdf_num = "0";
    test_invoice.setInvoiceID("INV-1001");
    test_invoice.set_file_name(pdf_out_path + pdf_num + ".pdf");
    test_invoice.setCurrentDate("2024-10-01");
    test_invoice.setDueDate("2024-10-15");
    test_invoice.setTotalAmount(500.0f);
    test_invoice.set_website("www.google.com");

    Infrastructure::PDF::InvoicePdfGenerator pdf_gen(std::make_shared<Invoice>(test_invoice));

    pdf_gen.peece_template();

    QVERIFY(fs::exists(pdf_out_path + pdf_num + ".pdf"));
}

QTEST_MAIN(TestPDFGen)
#include "PDFGenTests.moc"
