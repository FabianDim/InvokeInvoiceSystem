#pragma once


class StockItem;

class SetStockItem {
	friend class StockManager;
public:
	static std::shared_ptr<StockItem> setStockItem(const std::string& stockID);
};