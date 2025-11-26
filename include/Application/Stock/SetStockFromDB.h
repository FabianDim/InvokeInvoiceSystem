#pragma once

#include "Domain/Stock/StockItem.h"

class SetStockItem {
	friend class StockManager;
public:
	static std::shared_ptr<StockItem> setStockItem(const std::string& stockID);
};