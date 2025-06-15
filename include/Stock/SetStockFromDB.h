#pragma once

#include "StockItem.h"

class SetStockItem {
	friend class StockManager;
private:
	static std::shared_ptr<StockItem> setStockItem(const std::string& stockID);
};