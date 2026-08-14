#include <QtTest/QtTest>
#include "Domain/Invoices/Invoice.h"
#include "Infrastructure/Pdf/InvoicePdfGenerator.h"
#include <cstdlib>
#include <queue>
#include <regex>
#include <vector>
namespace fs = std::filesystem;
class TestPDFGen : public QObject {
    Q_OBJECT
  private slots:
    void pdf_creation_test();
    void pdf_multiple_page_creation_test();
    void pdf_three_long_lines();

  private:
    std::string delete_old_pdfs(fs::path dir);
    const std::string pdf_out_path = "G:/misc/Test_Invoices/test_invoice";
    const std::string pdf_out_path_multi = "G:/misc/Test_Invoices/test_invoice_multi";
    const std::string pdf_out_path_long = "G:/misc/Test_Invoices/test_invoice_long";
};

void TestPDFGen::pdf_multiple_page_creation_test() {
    Invoice test_invoice;
    std::shared_ptr<BusinessRepository> biz = std::make_shared<BusinessRepository>();
    biz->set_website_url("https://google.com");
    biz->setName("My Business");
    QString logoPath = QCoreApplication::applicationDirPath() + "../../../../tests/test_resources/logo.png";
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

    // every ten test delete the oldest 5 tests.
    fs::path dir = "G:/misc/Test_Invoices";

    const std::string pdf_num = delete_old_pdfs(dir);
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
    QString logoPath = QCoreApplication::applicationDirPath() + "../../../../tests/test_resources/logo.png";
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

    // every ten test delete the oldest 5 tests.
    fs::path dir = "G:/misc/Test_Invoices";

    const std::string pdf_num = delete_old_pdfs(dir);
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
    QString logoPath = QCoreApplication::applicationDirPath() + "../../../../tests/test_resources/logo.png";
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
    // every ten test delete the oldest 5 tests.
    fs::path dir = "G:/misc/Test_Invoices";

    const std::string pdf_num = delete_old_pdfs(dir);
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

std::string TestPDFGen::delete_old_pdfs(fs::path dir) {
    std::cout << "Deleting old PDFs from: " << dir.string() << std::endl;
    std::deque<std::pair<int, fs::path>> dq;
    int max = INT_MIN;
    int new_file_number = 0;
    if (fs::exists(dir) && fs::is_directory(dir)) {
        std::cout << dir.string() << " exists." << std::endl;
        for (const auto& entry : fs::directory_iterator(dir)) {
            std::string file_name = entry.path().string();
            if (!file_name.empty()) {
                std::string file_num =
                    std::regex_replace(file_name, std::regex("[^0-9]*([0-9]+).*"), std::string("$1"));
                try {
                    if (stoi(file_num) > max) {
                        max = stoi(file_num);
                    }
                } catch (...) {
                    max = 0;
                    file_num = "0";
                }
                if (dq.size() <= 5) {
                    dq.push_back(std::make_pair(std::stoi(file_num), entry));
                    std::cout << "file num = " << file_num << std::endl;
                }
            }
        }
        std::cout << "most recent file number: " << max << std::endl;
        new_file_number = max + 1;
        if (dq.size() >= 5) {
            std::sort(dq.begin(), dq.end());
            while (!dq.empty()) {
                std::cout << "deleting: " << dq.front().second << std::endl;
                fs::remove(dq.front().second);
                dq.pop_front();
            }
        }
        return std::to_string(new_file_number > 0 ? new_file_number : 0);
    }
    return "0";
}
