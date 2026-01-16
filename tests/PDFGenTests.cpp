#include <QtTest/QtTest>
#include "Domain/Invoices/Invoice.h"
#include "Infrastructure/Pdf/InvoicePdfGenerator.h"
#include <cstdlib>
namespace fs = std::filesystem;
class TestPDFGen : public QObject {
    Q_OBJECT
  private slots:
    void pdf_creation_test();
};

void TestPDFGen::pdf_creation_test() {

    Invoice test_invoice;
    std::shared_ptr<BusinessRepository> biz = std::make_shared<BusinessRepository>();
    const std::string& name = "My Business";
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

    test_invoice.setInvoiceID("INV-1001");
    test_invoice.set_file_name("C:/Users/Fabian/Test_Invoices/test_invoice.pdf");
    test_invoice.setCurrentDate("2024-10-01");
    test_invoice.setDueDate("2024-10-15");
    test_invoice.setTotalAmount(500.0f);

    Infrastructure::PDF::InvoicePdfGenerator pdf_gen(std::make_shared<Invoice>(test_invoice));

    pdf_gen.peece_template();

    QVERIFY(fs::exists("C:/Users/Fabian/Desktop/test_invoice.pdf"));
}

QTEST_MAIN(TestPDFGen)
#include "PDFGenTests.moc"
