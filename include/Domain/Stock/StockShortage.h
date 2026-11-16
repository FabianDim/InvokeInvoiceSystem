#pragma once
#include <QJsonArray>
#include <QString>
#include <stdexcept>
#include <utility>

namespace Domain::Stock {
inline bool valid_shortage_policy(const QString& policy) {
    return policy == "reject" || policy == "zero" || policy == "negative";
}

class StockShortage : public std::runtime_error {
  public:
    explicit StockShortage(QJsonArray items)
        : std::runtime_error("Some invoice quantities exceed the available stock."), items(std::move(items)) {}
    QJsonArray items;
};
} // namespace Domain::Stock
