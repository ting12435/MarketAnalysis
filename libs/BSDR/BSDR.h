#ifndef BSDR_H
#define BSDR_H

#include <iostream>
// #include <experimental/filesystem>
// #include <filesystem> // c++ 17
#include <string>
#include <map>
#include <vector>
// #include <ostream>

#include "util/util.h"
#include "util/market_util.h"


// namespace fs = std::experimental::filesystem;
// namespace fs = std::filesystem; // c++ 17

class BSDR;

struct trade_detail {
	int b_lot;
	int s_lot;
	long b_amount;
	long s_amount;
};

typedef std::map<std::string, std::map<std::string, std::map<std::string, struct trade_detail>>> bsdr_date_issuer_stock_t;  // <K:date_str, <K: issuer, V:<K: stock, V: detail>>>
typedef std::map<std::string, std::map<std::string, std::map<std::string, struct trade_detail>>> bsdr_date_stock_issuer_t;  // <K:date_str, <K: stock, V:<K: issuer, V: detail>>>
typedef std::map<std::string, std::map<std::string, std::map<std::string, struct trade_detail>>> bsdr_issuer_date_stock_t;  // <K:issuer, <K: date_str, V:<K: stock, V: detail>>>
typedef std::map<std::string, std::map<std::string, std::map<std::string, struct trade_detail>>> bsdr_issuer_stock_date_t;  // <K:issuer, <K: stock, V:<K: date_str, V: detail>>>
typedef std::map<std::string, std::map<std::string, std::map<std::string, struct trade_detail>>> bsdr_stock_issuer_date_t;  // <K:stock, <K: issuer, V:<K: date_str, V: detail>>>
typedef std::map<std::string, std::map<std::string, std::map<std::string, struct trade_detail>>> bsdr_stock_date_issuer_t;  // <K:stock, <K: date_str, V:<K: issuer, V: detail>>>

typedef std::map<std::string, std::vector<BSDR*>> bsdr_data_t;  // K: date_str

class BSDR_record {
public:
	friend std::ostream& operator<<(std::ostream&, const BSDR_record&);

	int seq;
	std::string issuer_name;
	double px;
	int b_lot;
	int s_lot;
};

class BSDR {
public:
	explicit BSDR();

	static bsdr_date_issuer_stock_t get_analysis_data_date_issuer_stock(Date st_date, Date ed_date, Market);
	static bsdr_date_stock_issuer_t get_analysis_data_date_stock_issuer(Date st_date, Date ed_date, Market);
	static bsdr_issuer_date_stock_t get_analysis_data_issuer_date_stock(Date st_date, Date ed_date, Market);
	static bsdr_issuer_stock_date_t get_analysis_data_issuer_stock_date(Date st_date, Date ed_date, Market);
	static bsdr_stock_issuer_date_t get_analysis_data_stock_issuer_date(Date st_date, Date ed_date, Market);
	static bsdr_stock_date_issuer_t get_analysis_data_stock_date_issuer(Date st_date, Date ed_date, Market);


	static bsdr_data_t get_data(Date st_date, Date ed_date, Market);
	static void tester();

	friend std::ostream& operator<<(std::ostream&, const BSDR&);

	std::string filename;
	time_t trade_date = 0;
	std::string stock_fc;
	// int trade_count;
	// long trade_amount;
	// int trade_lot;
	// int open_px;
	// int high_px;
	// int low_px;
	// int close_px;
	// std::map<std::string, std::vector<struct trade_info_by_px>> issuers;
	std::vector<BSDR_record*> records;
	
private:
};




#endif // BSDR_H



