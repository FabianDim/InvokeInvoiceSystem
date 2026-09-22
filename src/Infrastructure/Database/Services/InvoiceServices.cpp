#include "Domain/Invoices/InvoiceTemplateEnum.h"
#include "Infrastructure/Database/Services/InvoiceServices.h"
#include <QJsonObject>
#include <functional>
#include <QJsonArray>
#include <stdexcept>
#include "Infrastructure/Pdf/InvoicePdfGenerator.h"
using namespace Infrastructure::Services;

InvoiceServices::InvoiceServices(MongoDBDataManager& db_manager) : db_manager_(db_manager) {}

void InvoiceServices::clear_session() {
    invoice_.clear_invoice();
    invoice_.set_file_name({});
}

bool InvoiceServices::has_invoice_details() const {
    return invoice_.getBusiness() && invoice_.getClient() && !invoice_.get_file_name().empty();
}
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
    clear_session();
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
    qDebug() << "Business added to invoice." << QString::fromStdString(biz->getBizName());
}

void InvoiceServices::add_client_to_invoice(const QJsonDocument& doc) {
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
    auto template_converter = [&](std::string s) -> InvoiceTemplateEnum {
        if (s == "PEECE")
            return InvoiceTemplateEnum::PEECE;
        return InvoiceTemplateEnum::PEECE;
    };
    invoice_.setCliInvoiceID(doc.object().value("invoice_number").toString().toStdString());
    invoice_.setDueDate(doc.object().value("date_due").toString().toStdString());
    invoice_.setCurrentDate(doc.object().value("date_created").toString().toStdString());
    invoice_.setTemplate(template_converter(doc.object().value("invoice_theme").toString().toStdString()));
    invoice_.set_file_name(doc.object().value("file_dir").toString().toStdString() + "/" +
                           normalise_file_name(doc.object().value("file_name").toString().toStdString()));
}

/**
 * @brief Add the list of stock items to the invoice object.
 *
 * Takes the json object passed through the api from the frontend
 * and creates stock objects and adds them to the current invoice object.
 *
 * @param doc A JSon object list of stock items
 * @return Void
 * @pre An invoice object should be created.
 */
std::string Infrastructure::Services::InvoiceServices::add_stock_to_invoice(const QJsonDocument& doc) {
    if (!has_invoice_details())
        throw std::invalid_argument("Choose a business and client and enter invoice details first.");
    if (!doc.isArray() || doc.array().isEmpty())
        throw std::invalid_argument("Add at least one stock item to the invoice.");
    const QJsonArray stock_array = doc.array();
    // A retry replaces the submitted items rather than duplicating the previous attempt.
    invoice_.getStockQuantityMap().clear();
    qDebug() << "Adding stock to invoices.\n";
    for (const auto& obj : stock_array) {
        const auto& stock_object = obj.toObject();
        if (stock_object.empty())
            throw std::invalid_argument("Invalid invoice stock item.");
        auto item = std::make_shared<StockItem>();
        item->setStockID(stock_object.value("StockID").toString().toStdString());
        item->set_description(stock_object["Name"].toString().toStdString());
        item->setStdPrice(stock_object["Price"].toDouble());
        invoice_.addStockItem(item, stock_object["Quantity"].toInt());
    }
    if (!build_invoice())
        throw std::runtime_error("Could not write the PDF. Check the output folder and try again.");
    return invoice_.get_file_name();
}

/**
 * @brief Create an instance of the PDF builder and build the Invoice.
 *
 * Uses the invoice object to build the InvoicePDFGenerator class and generate
 * the invoice in a specified format.
 *
 * @return Bool flag of successful completion
 * @pre An invoice object should be created.
 */
bool Infrastructure::Services::InvoiceServices::build_invoice() {
    Infrastructure::PDF::InvoicePdfGenerator pdf_gen(std::make_shared<Invoice>(invoice_));
    return pdf_gen.peece_template();
}

/**
 * @brief Takes the file name and normalises it to not have spaces and end with pdf.
 *
 *
 * @param file_name the raw file name from the front end.
 * @return String with normalised name
 */
std::string Infrastructure::Services::InvoiceServices::normalise_file_name(const std::string& file_name) {
    std::string new_file_name = file_name;
    std::replace(new_file_name.begin(), new_file_name.end(), ' ', '_');
    if (!new_file_name.ends_with(".pdf")) {
        new_file_name.append(".pdf");
    }
    return new_file_name;
}
