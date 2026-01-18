#include <QtTest/QtTest>
#include "Domain/Invoices/Invoice.h"
#include "Infrastructure/Pdf/InvoicePdfGenerator.h"
#include <cstdlib>
#include <regex>
#include <queue>
namespace fs = std::filesystem;
class TestPDFGen : public QObject {
    Q_OBJECT
  private slots:
    void pdf_creation_test();

  private:
    std::string delete_old_pdfs(fs::path dir);
};

void TestPDFGen::pdf_creation_test() {

    Invoice test_invoice;
    std::shared_ptr<BusinessRepository> biz = std::make_shared<BusinessRepository>();
    const std::string name = "My Business";
    biz->setName(name);
    test_invoice.setBusiness(biz);

    StockItem items[5];

    StockItem item1;
    StockItem item2;
    StockItem item3;
    StockItem item4;
    StockItem item5;

    item1.setName("Item One");
    item1.setStdPrice(10.0f);
    item1.setStockOnHand(100);

    item2.setName("Item Two");
    item2.setStdPrice(20.0f);
    item2.setStockOnHand(200);

    item3.setName("Item Three");
    item3.setStdPrice(30.0f);
    item3.setStockOnHand(300);

    item4.setName("Item Four");
    item4.setStdPrice(40.0f);
    item4.setStockOnHand(400);

    item5.setName("Item Five");
    item5.setStdPrice(50.0f);
    item5.setStockOnHand(500);

    items[0] = item1;
    items[1] = item2;
    items[2] = item3;
    items[3] = item4;
    items[4] = item5;

    test_invoice.addStockItem(std::make_shared<StockItem>(items[0]), 2);
    test_invoice.addStockItem(std::make_shared<StockItem>(items[1]), 3);
    test_invoice.addStockItem(std::make_shared<StockItem>(items[2]), 1);
    test_invoice.addStockItem(std::make_shared<StockItem>(items[3]), 5);
    test_invoice.addStockItem(std::make_shared<StockItem>(items[4]), 4);
    // every ten test delete the oldest 5 tests.
    fs::path dir = "F:/misc/Test_Invoices";

    const std::string pdf_num = delete_old_pdfs(dir);
    test_invoice.setInvoiceID("INV-1001");
    test_invoice.set_file_name("F:/misc/Test_Invoices/test_invoice" + pdf_num + ".pdf");
    test_invoice.setCurrentDate("2024-10-01");
    test_invoice.setDueDate("2024-10-15");
    test_invoice.setTotalAmount(500.0f);

    Infrastructure::PDF::InvoicePdfGenerator pdf_gen(std::make_shared<Invoice>(test_invoice));

    pdf_gen.peece_template();

    QVERIFY(fs::exists("F:/misc/Test_Invoices/test_invoice" + pdf_num + ".pdf"));
}

QTEST_MAIN(TestPDFGen)
#include "PDFGenTests.moc"

std::string TestPDFGen::delete_old_pdfs(fs::path dir) {
    std::cout << "Deleting old PDFs from: " << dir.string() << std::endl;
    std::priority_queue<std::pair<int, fs::path>> dq;
    int max = INT_MIN;
    int new_file_number = 0;
    if (fs::exists(dir) && fs::is_directory(dir)) {
        std::cout << dir.string() << " exists." << std::endl;
        for (const auto& entry : fs::directory_iterator(dir)) {
            std::string file_name = entry.path().string();
            if (!file_name.empty()) {
                std::string file_num =
                    std::regex_replace(file_name, std::regex("[^0-9]*([0-9]+).*"), std::string("$1"));

                if (stoi(file_num) > max) {
                    max = stoi(file_num);
                }
                if (dq.size() <= 5) {
                    dq.push(std::make_pair(std::stoi(file_num), entry));
                    std::cout << "file num = " << file_num << std::endl;
                }
                while (dq.size() >= 5) {
                    std::cout << "popping: " << dq.top().first << std::endl;
                    dq.pop();
                }
            }
        }
        std::cout << "most recent file number: " << max << std::endl;
        new_file_number = max + 1;
        if (dq.size() >= 5) {
            while (!dq.empty()) {
                std::cout << "deleting: " << dq.top().second << std::endl;
                fs::remove(dq.top().second);
                dq.pop();
            }
        }
        return std::to_string(new_file_number);
    }
    return "";
}
