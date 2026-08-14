#include "Application/Invoices/SetInvoiceFromDB.h"
#include "bsoncxx/exception/exception.hpp"
std::shared_ptr<Invoice> SetInvoice::setInvoice(const std::string invoiceID) {
    MongoDBDataManager dbManager;
    auto result = dbManager.findOne("Invoices", make_document(kvp("InvoiceID", invoiceID)));

    try {
        if (result) {
            auto view = result->view();
            auto invoice = std::make_shared<Invoice>();

            // Extract and convert fields
            std::string invoiceIDStr{ view["InvoiceID"].get_string().value };
            std::string clientInvoice{ view["ClientInvoiceID"].get_string().value };
            std::string clientID{ view["ClientID"].get_string().value };
            std::string invoiceDate{ view["InvoiceDate"].get_string().value };
            std::string dueDate{ view["DueDate"].get_string().value };
            bool isPaid = view["IsPaid"].get_bool().value;
            bool gstIncluded = view["GSTIncluded"].get_bool().value;
            std::string invoiceTemplate{ view["InvoiceTemplate"].get_string().value };
            std::string notes{ view["Notes"].get_string().value };

            // Assign to invoice
            invoice->setInvoiceID(invoiceIDStr);
            invoice->setCliInvoiceID(clientInvoice);
            invoice->setCurrentDate(invoiceDate);
            invoice->setDueDate(dueDate);
            invoice->setIsPaid(isPaid);
            invoice->setTaxAmount(gstIncluded);
            invoice->setTemplate(to_enum(invoiceTemplate));
            invoice->setNotes(notes);

            // Get and set client (via manager)
             //use set client from db to make a new client object
            //add that to the shir
            auto client = setClient.setClientFromDB(clientID);
            if (client) {
                invoice->setClient(client);
            }
            else {
                std::cerr << "Client not found: " << clientID << std::endl;
            }

            // Process StockItems array
            std::unordered_map<std::shared_ptr<StockItem>, int> stockMap;
            auto stockArray = view["StockItems"].get_array().value;
            for (const auto& elem : stockArray) {
                auto stockObj = elem.get_document().value;

                std::string stockID{ stockObj["StockID"].get_string().value };
                int quantity = stockObj["Quantity"].get_int32().value;

                auto stockItem = SetStockItem::setStockItem(stockID);
                if (stockItem) {
                    stockMap[stockItem] = quantity;
                }
                else {
                    std::cerr << "StockItem not found for ID: " << stockID << std::endl;
                }
            }
            invoice->setStockQuantityMap(stockMap);

            return invoice;
        }
        else {
            std::cerr << "Invoice not found for ID: " << invoiceID << std::endl;
        }
    }
    catch (const mongocxx::exception& e) {
        std::cout << "MongoDB Exception: " << e.what() << std::endl;
        return nullptr;
    }
    catch (const bsoncxx::exception& e) {
        std::cout << "Tried to parse a wrong JSON:\n" << e.what() << std::endl;
        std::cout << "------------------------------------- end error message" << std::endl;
    }

    return nullptr;
}
