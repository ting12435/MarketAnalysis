#ifndef MI_INDEX_H
#define MI_INDEX_H

#include <iostream>
#include <filesystem>
#include <string>
#include <map>
#include <vector>

#include "util/util.h"
#include "util/market_util.h"

namespace fs = std::filesystem;

class MI_Index_Data;

typedef std::map<std::string, MI_Index_Data*> mi_index_data_t;  // K: date_str


class MI_Index {
	static mi_index_data_t get_data(Date st_date, Date ed_date);
};


class MarketStatistics {

	class trade_status {
	public:
		long trade_amount;
		long trade_lot;
		long trade_count;
	};

	std::map<security_type, trade_status> data;
};

class StockDaily {

	class trade_status {
	public:
		long trade_amount;
		long trade_lot;
		long trade_count;

		int px100_o;
		int px100_h;
		int px100_l;
		int px100_c;

		int px_diff_status;
		int px_diff;

		int last_bid_px100;
		int last_bid_lot;
		int last_ask_px100;
		int last_ask_lot;

		double pe_ratio;
	};

	std::map<std::string, trade_status> data;
};

class MI_Index_Data {
public:
	std::string filename;
	Date date;

	MarketStatistics market_statistics;
	StockDaily stock_daily;

private:
	MI_Index_Data* read_file(fs::directory_entry);
};


#endif // MI_INDEX_H