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
    QJsonArray invoices;
    QJsonObject invoice_list(const QString& business_id, int offset = 0) const;
    QJsonObject saved_invoice(const QString& business_id, const QString& id) const;
    void set_invoice_paid(const QString& business_id, const QString& id, bool paid);

    QString save(const QString& resource, QJsonObject data, const QString& business_id);
    void update_stock(const QString& business_id, const QJsonObject& data);
    QJsonObject items(const QString& business_id) const;
    void clear_invoice();
    void begin_invoice(QJsonObject details, const QString& business_id);
    QString generate_pdf(const QJsonArray& stock, const QString& shortage_policy = "reject");
    QString invoice_number() const { return invoice_details_.value("invoice_number").toString(); }

  private:
    quint64 next_id_ = 0;
    QJsonArray completed_items_;
    bool completed_ = false;
    QString completed_id_;
    QJsonObject invoice_details_;
    QJsonObject invoice_business_;
    QJsonObject invoice_client_;
};
} // namespace Application::Invoices
