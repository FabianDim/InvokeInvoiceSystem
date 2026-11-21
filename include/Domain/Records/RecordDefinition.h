#pragma once
#include "Domain/Stock/StockEdit.h"
#include "Domain/Business/InvoicePreferences.h"
#include "Utils/BusinessLogo.h"
#include <QUrl>
#include <QVector>

namespace Domain::Records {
enum class FieldKind { Text, Quantity, Money, Percent, Keywords, Website, Address };

struct RecordField {
    QString key;
    QString form_name;
    QString label;
    QString storage_name;
    FieldKind kind = FieldKind::Text;
    bool required = false;
    int maximum_length = 200;
};

// The shared contract used by the creation form, editor and persistence layer.
// Register supported record types explicitly: arbitrary JSON keys are not editable.
class RecordDefinition {
  public:
    static const RecordDefinition& for_resource(const QString& resource) {
        static const RecordDefinition business{"business", "business", "Business", {
            {"ABN", "ABN", "ABN", "ABN"},
            {"BusinessName", "Business name", "Business name", "BusinessName", FieldKind::Text, true},
            {"Phone", "Business phone", "Phone", "Phone"},
            {"BusinessAddress", "Address", "Address", "BusinessAddress", FieldKind::Address, false, 2000},
            {"ACN", "ACN", "ACN", "ACN"},
            {"Website", "Website", "Website", "Website", FieldKind::Website, false, 2000}}};
        static const RecordDefinition client{"client", "client", "Clients", {
            {"Name", "Name", "Name", "ClientName", FieldKind::Text, true},
            {"Phone", "Phone", "Phone", "Phone"},
            {"Email", "Email", "Email", "Email"},
            {"Address", "Address", "Address", "Address", FieldKind::Address, false, 2000}}};
        static const RecordDefinition stock{"stock", "stock item", "Stock", {
            {"Name", "Name", "Name", "Name", FieldKind::Text, true},
            {"Quantity", "Quantity", "Stock on hand", "Quantity", FieldKind::Quantity},
            {"Price", "Price", "Price (excluding GST)", "StdPrice", FieldKind::Money, true},
            {"Margin", "Margin", "Margin (%)", "ProfitMargin", FieldKind::Percent},
            {"Keywords", "Keywords", "Keywords (comma-separated)", "ProductKeywords", FieldKind::Keywords, false, 2000},
            {"Unit", "Unit", "Unit", "Unit Type", FieldKind::Text, false, 80}}};
        if (resource == business.resource_) return business;
        if (resource == client.resource_) return client;
        if (resource == stock.resource_) return stock;
        throw std::invalid_argument("Choose a business, client or stock record.");
    }

    const QString& resource() const { return resource_; }
    const QString& noun() const { return noun_; }
    const QString& collection() const { return collection_; }
    const QVector<RecordField>& fields() const { return fields_; }
    bool is_business() const { return resource_ == "business"; }
    bool tracks_quantity() const { return resource_ == "stock"; }

    const RecordField* field(const QString& name) const {
        for (const auto& field : fields_)
            if (field.form_name == name || field.key == name) return &field;
        return nullptr;
    }
    QString storage_name(const QString& key) const {
        const auto* entry = field(key);
        return entry ? entry->storage_name : key;
    }
    QVector<QString> form_fields(bool editing = false) const {
        QVector<QString> result;
        for (const auto& entry : fields_) {
            if (entry.kind == FieldKind::Address && !editing)
                result += QVector<QString>{"Country", "State or province", "City", "Street address", "Postcode"};
            else result.append(entry.form_name);
        }
        if (is_business()) result.append("Are You Registered for GST");
        return result;
    }
    QJsonObject canonical_values(const QJsonObject& form) const {
        QJsonObject result = form;
        for (const auto& entry : fields_) result[entry.key] = form.value(entry.form_name);
        return result;
    }
    QJsonObject edit_values(const QJsonObject& data) const {
        if (tracks_quantity()) return Stock::stock_edit_values(data);
        QJsonObject result;
        for (const auto& entry : fields_) {
            const auto value = data.value(entry.key);
            if (!value.isString() || value.toString().size() > entry.maximum_length)
                throw std::invalid_argument(QString("Enter %1 as text of at most %2 characters.")
                    .arg(entry.label.toLower()).arg(entry.maximum_length).toStdString());
            auto text = value.toString().trimmed();
            if (entry.required && text.isEmpty()) throw std::invalid_argument("Enter a name.");
            if (entry.kind == FieldKind::Website && !text.isEmpty()) {
                const auto url = QUrl::fromUserInput(text);
                if (!url.isValid() || url.host().isEmpty() || (url.scheme() != "https" && url.scheme() != "http"))
                    throw std::invalid_argument("Enter a valid business website, such as https://example.com.");
                text = url.toString();
            }
            result[entry.key] = text;
        }
        if (is_business()) {
            if (!Business::valid_preferences(data)) throw std::invalid_argument("Invalid GST, payment terms or bank details.");
            for (const auto* key : {"GSTRegistered", "PaymentTerms", "BankName", "BankAccountName", "BankBSB", "BankAccountNumber"})
                if (data.contains(key)) result[key] = data.value(key);
            if (data.contains("LogoData")) {
                if (!data.value("LogoData").isString()) throw std::invalid_argument("Invalid business logo.");
                const auto encoded = data.value("LogoData").toString().toLatin1();
                QByteArray png;
                QString error;
                if (!encoded.isEmpty() && (encoded.size() > (BusinessLogo::maximum_bytes + 2) / 3 * 4 ||
                    !BusinessLogo::normalise(QByteArray::fromBase64(encoded), png, error)))
                    throw std::invalid_argument("Choose a valid PNG or JPEG logo no larger than 5 MB.");
                result["LogoData"] = QString::fromLatin1(png.toBase64());
            }
        }
        return result;
    }

  private:
    RecordDefinition(QString resource, QString noun, QString collection, QVector<RecordField> fields)
        : resource_(std::move(resource)), noun_(std::move(noun)), collection_(std::move(collection)), fields_(std::move(fields)) {}
    QString resource_;
    QString noun_;
    QString collection_;
    QVector<RecordField> fields_;
};

class RecordUpdate {
  public:
    RecordUpdate(const QString& resource, const QJsonObject& data)
        : definition_(RecordDefinition::for_resource(resource)), data_(data), values_(definition_.edit_values(data)) {}
    const RecordDefinition& definition() const { return definition_; }
    const QJsonObject& data() const { return data_; }
    const QJsonObject& values() const { return values_; }
    QJsonObject apply_to(QJsonObject record) const {
        if (definition_.tracks_quantity() && record.value("Quantity") != data_.value("ExpectedQuantity"))
            throw std::invalid_argument("Stock on hand changed. Cancel, refresh records and reopen the item before saving.");
        for (auto it = values_.begin(); it != values_.end(); ++it) record[it.key()] = it.value();
        if (definition_.is_business()) {
            record["Business name"] = record.value("BusinessName");
            record["Business phone"] = record.value("Phone");
            record["Address"] = record.value("BusinessAddress");
        }
        return record;
    }
  private:
    const RecordDefinition& definition_;
    QJsonObject data_;
    QJsonObject values_;
};
} // namespace Domain::Records
