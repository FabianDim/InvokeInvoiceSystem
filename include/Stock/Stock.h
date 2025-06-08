#pragma once
#include "pch.h"
class Stock {
private:
	std::string profitMargin;
	std::string name;
	float stdPrice;
	int stockOnHand;
	float profitMargin;
public:
	//getters;

	std::string getStockID() { return stockId };
	std::string getName() { return name };
	float getStdPrice() { return stdPrice };
	int getStockID() { return stockOnHand };
	float getProfitMargin() { return stockId };

	//setters;

	void setStockID(const std::string& id) { stockId = id; }
	void setName(const std::string& n) { name = n; }
	void setStdPrice(float price) { stdPrice = price; }
	void setStockOnHand(int onHand) { stockOnHand = onHand; }
	void setProfitMargin(float margin) { profitMargin = margin; }
};