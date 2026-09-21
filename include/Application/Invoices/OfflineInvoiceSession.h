#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QString>

namespace Application::Invoices {
// A demo owns only newly entered records. It has no database or HTTP dependency.
class OfflineInvoiceSession {
  public:
    QJsonObject businesses;
    QJsonArray clients;
    QJsonArray stocks;

    QString save(const QString& resource, QJsonObject data, const QString& business_id);
    QJsonObject items(const QString& business_id) const;
    void clear_invoice();
    void begin_invoice(QJsonObject details, const QString& business_id);
    QString generate_pdf(const QJsonArray& stock) const;

  private:
    QJsonObject invoice_details_;
    QJsonObject invoice_business_;
    QJsonObject invoice_client_;
};
} // namespace Application::Invoices
