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

public:
	// Getters
	std::string getStockID() const { return stockId; }
	std::string getBusinessID() { return bizID; }
	std::string getName() const { return name; }
	float getStdPrice() const { return stdPrice; }
	int getStockOnHand() const { return stockOnHand; }
	float getProfitMargin() const { return profitMargin; }

	// Setters
	void setStockID(const std::string& id) { stockId = id; }
	void setBusinessID(const std::string& biz) { bizID = biz; }
	void setName(const std::string& n) { name = n; }
	void setStdPrice(float price) { stdPrice = price; }
	void setStockOnHand(int onHand) { stockOnHand = onHand; }
	void setProfitMargin(float margin) { profitMargin = margin; }
};