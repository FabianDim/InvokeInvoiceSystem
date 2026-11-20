#pragma once
#include <QJsonObject>
#include <QJsonArray>
#include <cmath>
#include <stdexcept>

namespace Domain::Stock {
// Return only editable fields. Identities and ownership are never part of the update.
inline QJsonObject stock_edit_values(const QJsonObject& data) {
    const auto name = data.value("Name").toString().trimmed();
    if (name.isEmpty() || name.size() > 200) throw std::invalid_argument("Enter a stock name of at most 200 characters.");
    QJsonObject result{{"Name", name}};
    for (const auto* field : {"Quantity", "Price", "Margin", "ExpectedQuantity"}) {
        const auto value = data.value(field);
        const auto number = value.toDouble();
        const bool quantity = QString(field).contains("Quantity");
        if (!value.isDouble() || !std::isfinite(number) ||
            (quantity ? (std::abs(number) > 1'000'000 || std::floor(number) != number) : number < 0))
            throw std::invalid_argument(quantity ? "Stock on hand must be a whole number between -1,000,000 and 1,000,000."
                                                : "Price and margin must be valid, non-negative numbers.");
        if (QString(field) != "ExpectedQuantity") result[field] = number;
    }
    if (!data.value("Unit").isString() || data.value("Unit").toString().size() > 80 ||
        !data.value("Keywords").isString() || data.value("Keywords").toString().size() > 2000)
        throw std::invalid_argument("Keep the unit within 80 characters and keywords within 2,000 characters.");
    const auto unit = data.value("Unit").toString().trimmed();
    result["Unit"] = unit.isEmpty() ? "each" : unit;
    QJsonArray keywords;
    for (const auto& word : data.value("Keywords").toString().split(',', Qt::SkipEmptyParts))
        if (!word.trimmed().isEmpty()) keywords.append(word.trimmed().toLower());
    result["Keywords"] = keywords;
    return result;
}
}
