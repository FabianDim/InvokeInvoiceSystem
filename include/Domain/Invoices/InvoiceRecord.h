#pragma once
#include "Domain/Invoices/Invoice.h"
#include <QDate>
#include <QJsonArray>
#include <QJsonObject>
#include <cmath>
#include <stdexcept>

namespace Domain::Invoices {
inline void validate_details(const QJsonObject& details) {
    for (const auto& key : {"date_created", "date_due"}) {
        if (details.contains(key) && (!details.value(key).isString() ||
            !QDate::fromString(details.value(key).toString(), Qt::ISODate).isValid()))
            throw std::invalid_argument("Enter valid invoice and due dates.");
    }
    if (details.contains("paid") && !details.value("paid").isBool())
        throw std::invalid_argument("Payment status must be paid or unpaid.");
}

inline QJsonObject record(const Invoice& invoice, const QJsonArray& items) {
    const auto business = invoice.getBusiness();
    const auto client = invoice.getClient();
    double total = 0;
    for (const auto& item : items) {
        const auto line = item.toObject();
        total += line.value("Quantity").toDouble() * line.value("Price").toDouble();
    }
    return {{"Version", 1}, {"InvoiceNumber", QString::fromStdString(invoice.getCliInvoiceID())},
        {"DateCreated", QString::fromStdString(invoice.getCurrentDate())},
        {"DateDue", QString::fromStdString(invoice.getDueDate())}, {"Paid", invoice.getIsPaid()},
        {"Total", total}, {"Items", items},
        {"Business", QJsonObject{{"_id", QString::fromStdString(business->getBizID())},
            {"BusinessName", QString::fromStdString(business->getBizName())},
            {"ABN", QString::fromStdString(business->getAbn())},
            {"ACN", QString::fromStdString(business->getAcn())},
            {"Address", QString::fromStdString(business->getAddress())},
            {"Phone", QString::fromStdString(business->getPhone())},
            {"Website", QString::fromStdString(business->get_website_url())},
            {"LogoData", QString::fromStdString(business->get_business_logo_data())}}},
        {"Client", QJsonObject{{"_id", QString::fromStdString(client->getClientID())},
            {"Name", QString::fromStdString(client->getName())},
            {"Address", QString::fromStdString(client->getAddress())},
            {"Email", QString::fromStdString(client->getEmail())},
            {"Phone", QString::fromStdString(client->getPhoneNumber())}}}};
}

inline std::shared_ptr<Invoice> restore(const QJsonObject& data) {
    auto invoice = std::make_shared<Invoice>();
    auto business = std::make_shared<BusinessRepository>();
    const auto biz = data.value("Business").toObject();
    business->setBizID(biz.value("_id").toString().toStdString());
    business->setBizName(biz.value("BusinessName").toString().toStdString());
    business->setAbn(biz.value("ABN").toString().toStdString());
    business->setAcn(biz.value("ACN").toString().toStdString());
    business->setAddress(biz.value("Address").toString().toStdString());
    business->setPhone(biz.value("Phone").toString().toStdString());
    business->set_website_url(biz.value("Website").toString().toStdString());
    business->set_business_logo_data(biz.value("LogoData").toString().toStdString());
    invoice->setBusiness(business);
    auto client = std::make_shared<Client>();
    const auto cli = data.value("Client").toObject();
    client->setClientID(cli.value("_id").toString().toStdString());
    client->setName(cli.value("Name").toString().toStdString());
    client->setAddress(cli.value("Address").toString().toStdString());
    client->setEmail(cli.value("Email").toString().toStdString());
    client->setPhoneNumber(cli.value("Phone").toString().toStdString());
    invoice->setClient(client);
    invoice->setCliInvoiceID(data.value("InvoiceNumber").toString().toStdString());
    invoice->setCurrentDate(data.value("DateCreated").toString().toStdString());
    invoice->setDueDate(data.value("DateDue").toString().toStdString());
    invoice->setIsPaid(data.value("Paid").toBool());
    if (data.value("LegacyCopy").toBool())
        invoice->setNotes("Legacy copy: current contact details; original dates were not saved.");
    const auto items = data.value("Items").toArray();
    if (items.isEmpty()) throw std::invalid_argument("This invoice has no saved items.");
    for (const auto& value : items) {
        const auto line = value.toObject();
        const auto quantity = line.value("Quantity").toDouble(-1);
        const auto price = line.value("Price").toDouble(-1);
        if (line.value("Name").toString().trimmed().isEmpty() || !std::isfinite(quantity) || quantity < 1 ||
            quantity > 1'000'000 || std::floor(quantity) != quantity || !std::isfinite(price) || price < 0 || price > 1e9)
            throw std::invalid_argument("This invoice contains invalid saved items.");
        auto item = std::make_shared<StockItem>();
        item->setStockID(line.value("_id").toString().toStdString());
        item->set_description(line.value("Name").toString().toStdString());
        item->setStdPrice(price);
        invoice->addStockItem(item, static_cast<int>(quantity));
    }
    return invoice;
}

inline QJsonObject summary(QJsonObject record) {
    record["ClientName"] = record.value("Client").toObject().value("Name");
    record.remove("Business");
    record.remove("Client");
    record.remove("Items");
    return record;
}
} // namespace Domain::Invoices
