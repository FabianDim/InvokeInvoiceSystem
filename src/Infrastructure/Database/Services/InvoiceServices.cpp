#include "Domain/Invoices/InvoiceTemplateEnum.h"
#include "Infrastructure/Database/Services/InvoiceServices.h"
#include <QJsonObject>
#include <functional>
#include <QJsonArray>

using namespace Infrastructure::Services;

InvoiceServices::InvoiceServices(MongoDBDataManager& db_manager) : db_manager_(db_manager) {}

bool InvoiceServices::save_invoice(QJsonDocument& doc) {
    QJsonObject obj = doc.object();
    try {
        db_manager_.get_account_businesses(obj.value("UserID").toString().toStdString());
    } catch (const std::exception& e) {
        return true;
    }
    return false;
}

void InvoiceServices::add_business_to_invoice(const QJsonDocument& doc) {
    invoice_.clear_invoice();
    auto biz = std::shared_ptr<BusinessRepository>(new BusinessRepository());
    QJsonObject obj = doc.object();
    qDebug() << "document: " << doc;
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

    invoice_.setBusiness(biz);
    qDebug() << "Business added to invoice." << QString::fromStdString(biz->getBizName());
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
void Infrastructure::Services::InvoiceServices::add_stock_to_invoice(const QJsonDocument& doc) {
    const QJsonArray stock_array = doc.array();
    qDebug() << "Adding stock to invoices.\n";
    for (const auto& obj : stock_array) {
        const auto& stock_object = obj.toObject();
        std::shared_ptr<StockItem> item;
        try {
            if (!stock_object.empty()) {
                item->setName(stock_object["Name"].toString().toStdString());
                qDebug() << stock_object["Name"].toString().toStdString();
                item->setStdPrice(stock_object["Price"].toDouble());

                invoice_.addStockItem(item, stock_object["Quantity"].toInt());
            }

        } catch (std::exception err) {
            std::cerr << err.what();
        }
    }
}