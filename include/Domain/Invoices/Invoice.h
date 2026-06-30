#pragma once
#include "pch.h"
#include "Application/Business/BusinessManager.h"
#include "Application/Business/BusinessRepository.h"
#include "Application/Business/Clients/ClientManager.h"
#include "Domain/Business/Clients/Client.h"
#include "Application/Stock/StockManager.h"
#include "Domain/Stock/ClientStockItem.h"
#include "Utils/DateUtil.h"
#include "Utils/TemplateUtil.h"
#include "InvoiceTemplateEnum.h"
class Invoice {
  private:
    std::string invoice_id_;
    std::string client_invoice_id_;
    std::shared_ptr<BusinessRepository> business_;
    std::shared_ptr<Client> client_;
    std::string current_date_;
    std::string due_date_;
    std::string file_name_;
    std::unordered_map<std::shared_ptr<StockItem>, int> stock_quantity_map_; // stockID to quantity
    InvoiceTemplateEnum invoice_template_;
    float total_amount_ = 0.0f;
    float tax_amount_ = 0.0f;
    float discount_amount_ = 0.0f;
    bool is_paid_ = false;
    bool gst_included_ = false;
    std::string notes_;
    std::string website_;
    // std::string invoiceTemplate;

  public:
    // invoiceID
    std::string getInvoiceID() const {
        return invoice_id_;
    }
    void setInvoiceID(const std::string& id) {
        invoice_id_ = id;
    }

    // file_name
    std::string get_file_name() const {
        return file_name_;
    }
    void set_file_name(const std::string& file_name) {
        file_name_ = file_name;
    }

    void set_website(const std::string& website) {
        website_ = website;
    }

    // clientInvoiceID
    std::string getCliInvoiceID() const {
        return client_invoice_id_;
    }
    void setCliInvoiceID(const std::string& id) {
        client_invoice_id_ = id;
    }

    // business
    std::shared_ptr<BusinessRepository> getBusiness() const {
        return business_;
    }
    void setBusiness(std::shared_ptr<BusinessRepository> biz) {
        business_ = biz;
    }

    // client
    std::shared_ptr<Client> getClient() const {
        return client_;
    }
    void setClient(std::shared_ptr<Client> cli) {
        client_ = cli;
    }

    // currentDate
    std::string getCurrentDate() const {
        return current_date_;
    }
    void setCurrentDate(const std::string& date) {
        current_date_ = date;
    }

    // dueDate
    std::string getDueDate() const {
        return due_date_;
    }
    void setDueDate(const std::string& date) {
        due_date_ = date;
    }

    // stockQuantityMap
    std::unordered_map<std::shared_ptr<StockItem>, int>& getStockQuantityMap() {
        return stock_quantity_map_;
    }
    const std::unordered_map<std::shared_ptr<StockItem>, int>& getStockQuantityMap() const {
        return stock_quantity_map_;
    }
    void setStockQuantityMap(const std::unordered_map<std::shared_ptr<StockItem>, int>& map) {
        stock_quantity_map_ = map;
    }
    void addStockItem(const std::shared_ptr<StockItem>& item, int quantity) {
        stock_quantity_map_[item] = quantity;
    }

    // totalAmount
    float getTotalAmount() const {
        return total_amount_;
    }
    void setTotalAmount(float total) {
        total_amount_ = total;
    }

    //// taxAmount
    // float getTaxAmount() const { return tax_amount_; }
    // void setTaxAmount(float tax) { tax_amount_ = tax; }

    // Template
    InvoiceTemplateEnum getTemplate() const {
        return invoice_template_;
    }
    void setTemplate(InvoiceTemplateEnum invTemplate) {
        invoice_template_ = invTemplate;
    }

    // UsingGST
    bool getTaxAmount() const {
        return gst_included_;
    }
    void setTaxAmount(bool gst) {
        gst_included_ = gst;
    }

    // discountAmount
    float getDiscountAmount() const {
        return discount_amount_;
    }
    void setDiscountAmount(float discount) {
        discount_amount_ = discount;
    }

    // isPaid
    bool getIsPaid() const {
        return is_paid_;
    }
    void setIsPaid(bool paid) {
        is_paid_ = paid;
    }

    // notes
    std::string getNotes() const {
        return notes_;
    }
    void setNotes(const std::string& n) {
        notes_ = n;
    }

    void clear_invoice() {
        invoice_id_.clear();
        client_invoice_id_.clear();
        business_ = nullptr;
        client_ = nullptr;
        current_date_.clear();
        due_date_.clear();
        stock_quantity_map_.clear();
        invoice_template_ = InvoiceTemplateEnum::PEECE;
        total_amount_ = 0.0f;
        tax_amount_ = 0.0f;
        discount_amount_ = 0.0f;
        is_paid_ = false;
        gst_included_ = false;
        notes_.clear();
        website_.clear();
    }
};
