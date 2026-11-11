#include "Infrastructure/Logging/Logging.h"
#include "Domain/Invoices/InvoiceTemplateEnum.h"
#include "Infrastructure/Database/Services/InvoiceServices.h"
#include <QJsonObject>
#include <functional>
#include <QJsonArray>
#include <QTemporaryDir>
#include <QFile>
#include <cmath>
#include <stdexcept>
#include "Infrastructure/Pdf/InvoicePdfGenerator.h"
using namespace Infrastructure::Services;

InvoiceServices::InvoiceServices(MongoDBDataManager& db_manager) : db_manager_(db_manager) {}
/**
 * @brief this shit does nothing shlawg.
 */
bool InvoiceServices::save_invoice(QJsonDocument& doc) {
    QJsonObject obj = doc.object();
    try {
        db_manager_.get_account_businesses(obj.value("UserID").toString().toStdString());
    } catch (const std::exception& e) {
        return false;
    }
    return true;
}

void InvoiceServices::add_business_to_invoice(const QJsonDocument& doc) {
    Infrastructure::Logging::Operation operation(Infrastructure::Logging::pdf(), "InvoiceServices.add_business_to_invoice");
    invoice_.clear_invoice();
    auto biz = std::shared_ptr<BusinessRepository>(new BusinessRepository());
    QJsonObject obj = doc.object();
    if (!obj["BusinessID"].isUndefined())
        biz->setBizID(obj["BusinessID"].toString().toStdString());
    if (!obj["ABN"].isUndefined())
        biz->setAbn(obj["ABN"].toString().toStdString());
    if (!obj["ACN"].isUndefined())
        biz->setAcn(obj["ACN"].toString().toStdString());
    if (!obj["BusinessName"].isUndefined())
        biz->setBizName(obj["BusinessName"].toString().toStdString());
    if (!obj["Phone"].isUndefined())
        biz->setPhone(obj["Phone"].toString().toStdString());
    biz->set_website_url(obj.value("Website").toString().toStdString());
    biz->set_business_logo_path(obj.value("LogoPath").toString().toStdString());
    biz->set_business_logo_data(obj.value("LogoData").toString().toStdString());

    invoice_.setBusiness(biz);
    qCDebug(Infrastructure::Logging::pdf) << "event=invoice.business.attached";
}

void InvoiceServices::add_client_to_invoice(const QJsonDocument& doc) {
    Infrastructure::Logging::Operation operation(Infrastructure::Logging::pdf(), "InvoiceServices.add_client_to_invoice");
    const auto data = doc.object();
    auto client = std::make_shared<Client>();
    client->setClientID(data.value("ClientID").toString().toStdString());
    client->setName(data.value("Name").toString().toStdString());
    client->setAddress(data.value("Address").toString().toStdString());
    client->setEmail(data.value("Email").toString().toStdString());
    client->setPhoneNumber(data.value("Phone").toString().toStdString());
    invoice_.setClient(client);
}

void Infrastructure::Services::InvoiceServices::begin_invoice_details(const QJsonDocument& doc) {
    Infrastructure::Logging::Operation operation(Infrastructure::Logging::pdf(), "InvoiceServices.begin_invoice_details");
    auto template_converter = [&](std::string s) -> InvoiceTemplateEnum {
        if (s == "PEECE")
            return InvoiceTemplateEnum::PEECE;
        return InvoiceTemplateEnum::PEECE;
    };
    invoice_.setCliInvoiceID(doc.object().value("invoice_number").toString().toStdString());
    invoice_.setDueDate(doc.object().value("date_due").toString().toStdString());
    invoice_.setCurrentDate(doc.object().value("date_created").toString().toStdString());
    invoice_.setTemplate(template_converter(doc.object().value("invoice_theme").toString().toStdString()));
}

// Generate in a backend-owned temporary folder. A desktop path is never a server path.
QByteArray InvoiceServices::generate_pdf(const QJsonDocument& stock) const {
    Infrastructure::Logging::Operation operation(Infrastructure::Logging::pdf(), "InvoiceServices.generate_pdf");
    if (!stock.isArray() || stock.array().isEmpty())
        throw std::invalid_argument("Add at least one item to the invoice.");
    if (!invoice_.getBusiness() || !invoice_.getClient())
        throw std::invalid_argument("Start an invoice with a business and client first.");

    auto invoice = std::make_shared<Invoice>(invoice_);
    // A retry renders the submitted items once, without appending to the previous attempt.
    invoice->setStockQuantityMap({});
    for (const auto& value : stock.array()) {
        const auto item_data = value.toObject();
        const auto name = item_data.value("Name").toString().trimmed();
        const auto quantity = item_data.value("Quantity").toDouble(-1);
        const auto price = item_data.value("Price").toDouble(-1);
        if (name.isEmpty() || !std::isfinite(quantity) || quantity < 1 || quantity > 1'000'000 ||
            std::floor(quantity) != quantity || !std::isfinite(price) || price < 0 || price > 1e9)
            throw std::invalid_argument("Each item needs a name, a positive whole quantity and a valid price.");
        auto item = std::make_shared<StockItem>();
        item->setStockID(item_data.value("StockID").toString().toStdString());
        item->set_description(name.toStdString());
        item->setStdPrice(price);
        invoice->addStockItem(item, static_cast<int>(quantity));
    }

    qCDebug(Infrastructure::Logging::pdf) << "event=invoice.items.validated count=" << stock.array().size();
    QTemporaryDir output;
    if (!output.isValid())
        throw std::runtime_error("Could not create a temporary PDF folder.");
    const auto path = output.filePath("invoice.pdf");
    invoice->set_file_name(path.toStdString());
    Infrastructure::PDF::InvoicePdfGenerator generator(invoice);
    if (!generator.peece_template())
        throw std::runtime_error("Could not generate the invoice PDF.");
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        throw std::runtime_error("Could not read the generated invoice PDF.");
    const auto pdf = file.readAll();
    if (file.error() != QFileDevice::NoError || !pdf.startsWith("%PDF-"))
        throw std::runtime_error("The generated invoice PDF is invalid.");
    qCDebug(Infrastructure::Logging::pdf) << "event=pdf.response.ready bytes=" << pdf.size();
    return pdf;
}
