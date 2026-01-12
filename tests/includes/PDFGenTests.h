#include <QtWidgets>
#include <QTest>
#include "Domain/Invoices/Invoice.h"
#include "Infrastructure/Pdf/InvoicePdfGenerator.h"
class TestPDFGen : public QObject {
    Q_OBJECT

  private slots:
    void pdf_creation_test();
};