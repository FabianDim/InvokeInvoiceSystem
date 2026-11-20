#pragma once
#include "Application/Business/BusinessRepository.h"
#include <QDate>
#include <QJsonObject>
#include <stdexcept>

namespace Domain::Business {
inline bool gst_registered(const QJsonObject& data) {
    return data.value("GSTRegistered").toBool(data.value("Are You Registered for GST").toBool());
}
inline bool valid_preferences(const QJsonObject& data) {
    for (const auto* key : {"GSTRegistered", "Are You Registered for GST"})
        if (data.contains(key) && !data.value(key).isBool()) return false;
    if (data.contains("PaymentTerms") && (!data.value("PaymentTerms").isString() ||
        (data.value("PaymentTerms") != "COD" && data.value("PaymentTerms") != "14" && data.value("PaymentTerms") != "30")))
        return false;
    for (const auto* key : {"BankName", "BankAccountName", "BankBSB", "BankAccountNumber"})
        if (data.contains(key) && (!data.value(key).isString() || data.value(key).toString().size() > 120 ||
            data.value(key).toString().contains('\n') || data.value(key).toString().contains('\r'))) return false;
    return true;
}
inline void load_preferences(BusinessRepository& business, const QJsonObject& data) {
    business.set_gst_registered(gst_registered(data));
    business.set_payment_terms(data.value("PaymentTerms").toString("COD").toStdString());
    business.set_bank_name(data.value("BankName").toString().toStdString());
    business.set_bank_account_name(data.value("BankAccountName").toString().toStdString());
    business.set_bank_bsb(data.value("BankBSB").toString().toStdString());
    business.set_bank_account_number(data.value("BankAccountNumber").toString().toStdString());
}
inline QJsonObject preferences(const BusinessRepository& business) {
    return {{"GSTRegistered", business.get_gst_registered()},
        {"PaymentTerms", QString::fromStdString(business.get_payment_terms())},
        {"BankName", QString::fromStdString(business.get_bank_name())},
        {"BankAccountName", QString::fromStdString(business.get_bank_account_name())},
        {"BankBSB", QString::fromStdString(business.get_bank_bsb())},
        {"BankAccountNumber", QString::fromStdString(business.get_bank_account_number())}};
}
inline QString due_date(const QString& issued, const QString& terms) {
    const auto date = QDate::fromString(issued, Qt::ISODate);
    if (!date.isValid()) throw std::invalid_argument("Enter a valid invoice date.");
    return date.addDays(terms == "14" ? 14 : terms == "30" ? 30 : 0).toString(Qt::ISODate);
}
}
