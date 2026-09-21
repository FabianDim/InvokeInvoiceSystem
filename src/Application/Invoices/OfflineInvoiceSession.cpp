#include "Application/Invoices/OfflineInvoiceSession.h"
#include "Infrastructure/Pdf/InvoicePdfGenerator.h"
#include <QDir>
#include <QFileInfo>
#include <QUuid>
#include <QRegularExpression>
#include <cmath>
#include <stdexcept>

using namespace Application::Invoices;

namespace {
QJsonArray for_business(const QJsonArray& records, const QString& id) {
    QJsonArray result;
    for (const auto& value : records)
        if (value.toObject().value("BusinessID").toString() == id)
            result.append(value);
    return result;
}

double number(const QJsonValue& value, bool optional = false) {
    if (optional && (value.isUndefined() || value.isNull() || (value.isString() && value.toString().trimmed().isEmpty())))
        return 0;
    bool valid = true;
    const auto result = value.isDouble() ? value.toDouble() : value.toString().toDouble(&valid);
    if (!valid || !std::isfinite(result) || result < 0)
        throw std::runtime_error("Enter valid, non-negative stock quantities and prices.");
    return result;
}
} // namespace

QString OfflineInvoiceSession::save(const QString& resource, QJsonObject data, const QString& business_id) {
    if (resource != "business" && resource != "client" && resource != "stock")
        throw std::runtime_error("Offline mode only supports new businesses, clients and stock.");
    if (resource != "business" && !businesses.contains(business_id))
        throw std::runtime_error("Create and select an offline business first.");

    const auto id = "DEMO-" + QUuid::createUuid().toString(QUuid::WithoutBraces).left(8).toUpper();
    if (resource == "business") {
        data["BusinessID"] = id;
        data["BusinessName"] = data.value("Business name");
        data["Phone"] = data.value("Business phone");
        businesses[id] = data;
    } else {
        data["BusinessID"] = business_id;
        if (resource == "client") {
            data["ClientID"] = id;
            clients.append(data);
        } else {
            data["StockID"] = id;
            data["Quantity"] = number(data.value("Quantity"), true);
            data["Price"] = number(data.value("Price"));
            data["Margin"] = number(data.value("Margin"), true);
            if (data.value("Unit").toString().trimmed().isEmpty())
                data["Unit"] = "each";
            QJsonArray keywords;
            for (const auto& word : data.value("Keywords").toString().split(',', Qt::SkipEmptyParts))
                keywords.append(word.trimmed());
            data["Keywords"] = keywords;
            stocks.append(data);
        }
    }
    return id;
}

QJsonObject OfflineInvoiceSession::items(const QString& business_id) const {
    return {{"BusinessID", business_id},
            {"businesses", businesses.contains(business_id) ? QJsonArray{businesses.value(business_id)} : QJsonArray{}},
            {"clients", for_business(clients, business_id)}, {"stocks", for_business(stocks, business_id)}};
}

void OfflineInvoiceSession::clear_invoice() {
    invoice_details_ = {};
    invoice_business_ = {};
    invoice_client_ = {};
}

void OfflineInvoiceSession::begin_invoice(QJsonObject details, const QString& business_id) {
    clear_invoice();
    if (!businesses.contains(business_id))
        throw std::runtime_error("Create and select an offline business first.");
    for (const auto& value : for_business(clients, business_id)) {
        if (value.toObject().value("ClientID") == details.value("ClientID"))
            invoice_client_ = value.toObject();
    }
    if (invoice_client_.isEmpty())
        throw std::runtime_error("Create and select a client for this offline business first.");
    const auto directory = details.value("file_dir").toString();
    auto name = details.value("file_name").toString().trimmed();
    if (name.isEmpty() || name.contains(QRegularExpression("[<>:\"/\\\\|?*]")) || name == "." || name == "..")
        throw std::runtime_error("Enter a file name without path separators or reserved characters.");
    if (directory.isEmpty() || !QFileInfo(directory).isDir() || !QFileInfo(directory).isWritable())
        throw std::runtime_error("Choose an existing, writable folder for the PDF.");
    if (details.value("invoice_number").toString().trimmed().isEmpty())
        throw std::runtime_error("Enter an invoice number.");
    name.replace(' ', '_');
    if (!name.endsWith(".pdf", Qt::CaseInsensitive))
        name += ".pdf";
    details["output_path"] = QDir(directory).filePath(name);
    invoice_details_ = details;
    invoice_business_ = businesses.value(business_id).toObject();
}

QString OfflineInvoiceSession::generate_pdf(const QJsonArray& stock) const {
    if (invoice_details_.isEmpty() || stock.isEmpty())
        throw std::runtime_error("Complete the invoice details and add at least one item.");
    auto invoice = std::make_shared<Invoice>();
    // Explicitly exclude any signed-in account from the demo's business model.
    auto business = std::make_shared<BusinessRepository>(nullptr);
    business->setBizID(invoice_business_.value("BusinessID").toString().toStdString());
    business->setBizName(invoice_business_.value("BusinessName").toString().toStdString());
    business->setAbn(invoice_business_.value("ABN").toString().toStdString());
    business->setAcn(invoice_business_.value("ACN").toString().toStdString());
    business->setPhone(invoice_business_.value("Phone").toString().toStdString());
    business->setAddress(invoice_business_.value("Address").toString().toStdString());
    business->set_website_url(invoice_business_.value("Website").toString().toStdString());
    business->set_business_logo_data(invoice_business_.value("LogoData").toString().toStdString());
    invoice->setBusiness(business);
    auto client = std::make_shared<Client>();
    client->setClientID(invoice_client_.value("ClientID").toString().toStdString());
    client->setName(invoice_client_.value("Name").toString().toStdString());
    client->setAddress(invoice_client_.value("Address").toString().toStdString());
    client->setEmail(invoice_client_.value("Email").toString().toStdString());
    client->setPhoneNumber(invoice_client_.value("Phone").toString().toStdString());
    invoice->setClient(client);
    invoice->setCliInvoiceID(invoice_details_.value("invoice_number").toString().toStdString());
    invoice->setCurrentDate(invoice_details_.value("date_created").toString().toStdString());
    invoice->setDueDate(invoice_details_.value("date_due").toString().toStdString());
    invoice->setTemplate(InvoiceTemplateEnum::PEECE);
    const auto path = invoice_details_.value("output_path").toString();
    invoice->set_file_name(path.toStdString());
    for (const auto& value : stock) {
        const auto data = value.toObject();
        const auto quantity = number(data.value("Quantity"));
        if (quantity < 1 || quantity > 1'000'000 || std::floor(quantity) != quantity || data.value("Name").toString().trimmed().isEmpty())
            throw std::runtime_error("Each invoice item needs a name and a positive whole quantity.");
        auto item = std::make_shared<StockItem>();
        item->set_description(data.value("Name").toString().toStdString());
        item->setStdPrice(number(data.value("Price")));
        invoice->addStockItem(item, static_cast<int>(quantity));
    }
    Infrastructure::PDF::InvoicePdfGenerator generator(invoice);
    if (!generator.peece_template())
        throw std::runtime_error("Could not save the PDF. Check the output folder and try again.");
    return path;
}
