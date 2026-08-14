#pragma once
#include "pch.h"

class StockItem {
  private:
    std::string stockId;
    std::string bizID;
    std::string name;
    float stdPrice;
    int stockOnHand;
    float profitMargin;
    int invoice_stock;

  public:
    // Getters
    std::string getStockID() const {
        return stockId;
    }
    std::string getBusinessID() {
        return bizID;
    }
    std::string getName() const {
        return name;
    }
    float getStdPrice() const {
        return stdPrice;
    }
    int getStockOnHand() const {
        return stockOnHand;
    }
    float getProfitMargin() const {
        return profitMargin;
    }
    int get_invoice_stock() const {
        return invoice_stock;
    }

    // Setters
    void setStockID(const std::string& id) {
        stockId = id;
    }
    void setBusinessID(const std::string& biz) {
        bizID = biz;
    }
    void set_description(const std::string& n) {
        name = n;
    }
    void setStdPrice(float price) {
        stdPrice = price;
    }
    void setStockOnHand(int onHand) {
        stockOnHand = onHand;
    }
    void setProfitMargin(float margin) {
        profitMargin = margin;
    }
    void set_invoice_stock(int stock) {
        invoice_stock = stock;
    }
};