#ifndef BSDR_H
#define BSDR_H

#include <iostream>
#include <filesystem>
#include <string>
#include <map>
#include <vector>
// #include <ostream>

#include "util/util.h"
#include "util/market_util.h"

namespace fs = std::filesystem;

class BSDR;

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

	// static BSDR* read_file(fs::directory_entry);
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



