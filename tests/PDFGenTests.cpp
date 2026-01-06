#include "./includes/PDFGenTests.h"

void TestPDFGen::pdf_creation_test() {
    QLineEdit lineEdit;

    QTest::keyClicks(&lineEdit, "hello world");

    QCOMPARE(lineEdit.text(), QString("hello world"));
}
QTEST_MAIN(TestPDFGen)