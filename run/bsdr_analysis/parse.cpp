#include <iostream>
#include <fstream>

#include "BSDR/BSDR.h"
#include "util/util.h"

void parse();
void parse_trade_detail();
void parse_one_issuer();
void parse_issuers();
void parse_debug(bsdr_data_t*);

int main(int argc, char *argv[]) {

	// Date *st_date, *ed_date;
	// bsdr_data_t *bsdr_data;

	// st_date = new Date("2021-02-17");
	// ed_date = new Date("2021-02-17");

	// bsdr_data = new bsdr_data_t;
	// BSDR::get_data(bsdr_data, st_date, ed_date, Market::ALL);

	
	// parse_debug(bsdr_data);

	// parse_one_issuer();
	parse_trade_detail();
	// parse_issuers();
	// parse();

	return 0;
}

void parse() {
	long b_amount_sum, s_amount_sum;
	bsdr_issuer_date_stock_t d = BSDR::get_analysis_data_issuer_date_stock(Date("2021-02-18"), Date("2021-02-18"), Market::ALL);

	for (const auto &issuer_d: d) {
		b_amount_sum = s_amount_sum = 0;
		// std::cout << issuer_d.first << ",";
		if (issuer_d.first == "")
		for (const auto &date_d: issuer_d.second) {
			std::cout << date_d.first << ",";
			for (const auto &stock_d: date_d.second) {
				b_amount_sum += stock_d.second.b_amount;
				s_amount_sum += stock_d.second.s_amount;
			}
		}
		std::cout << b_amount_sum << "," << s_amount_sum << std::endl;
	}
}

void parse_trade_detail() {
	struct trade_detail {
		int b_lot;
		int s_lot;
		long b_amount;
		long s_amount;
	};

	const std::string issuer_name = "5850";
	// bsdr_data_t bsdr_data = BSDR::get_data(Date("2021-01-01"), Date("2021-02-18"), Market::ALL);
	// std::map<std::string, std::map<std::string, std::map<std::string, struct trade_detail>>> d;  // <K:date_str, <K: issuer, V:<K: stock, V: detail>>>

	// for (const auto &bsdr_d: bsdr_data) {
	// 	for (const auto &bsdr: bsdr_d.second) {
	// 		for (const auto &record: bsdr->records) {
	// 			d[bsdr_d.first][record->issuer_name][bsdr->stock_fc].b_lot += record->b_lot;
	// 			d[bsdr_d.first][record->issuer_name][bsdr->stock_fc].s_lot += record->s_lot;
	// 			d[bsdr_d.first][record->issuer_name][bsdr->stock_fc].b_amount += record->b_lot * record->px;
	// 			d[bsdr_d.first][record->issuer_name][bsdr->stock_fc].s_amount += record->s_lot * record->px;
	// 		}
	// 	}
	// }
	bsdr_date_issuer_stock_t d = BSDR::get_analysis_data_date_issuer_stock(Date("2021-04-01"), Date("2021-04-13"), Market::ALL);

	// output
	for (const auto &date_d: d) {
		OUTPUT("output/" + date_d.first);
		std::ofstream my_file("output/" + date_d.first);
		for (const auto &issuer_d: date_d.second) {

			if (issuer_d.first == issuer_name) {
				for (const auto &stock_d: issuer_d.second) {
					my_file << "=\"" << stock_d.first << "\"," << stock_d.second.b_lot << "," << stock_d.second.s_lot << "," << stock_d.second.b_lot - stock_d.second.s_lot << "," << stock_d.second.b_amount - stock_d.second.s_amount << std::endl;
				}
			}
		}
		my_file.close();
	}

	OUTPUT("output/all");
	std::ofstream my_file("output/all");
	std::map<std::string, struct trade_detail> m;  // K: stock
	for (const auto &date_d: d) {
		for (const auto &issuer_d: date_d.second) {
			if (issuer_d.first == issuer_name) {
				for (const auto &stock_d: issuer_d.second) {
					m[stock_d.first].b_lot += stock_d.second.b_lot;
					m[stock_d.first].s_lot += stock_d.second.s_lot;
					m[stock_d.first].b_amount += stock_d.second.b_amount;
					m[stock_d.first].s_amount += stock_d.second.s_amount;
				}
			}
		}
	}
	for (const auto &s: m) {
		my_file << "=\"" << s.first << "\"," << s.second.b_lot << "," << s.second.s_lot << "," << s.second.b_lot - s.second.s_lot << "," << s.second.b_amount - s.second.s_amount << std::endl;
	}
	my_file.close();
}

void parse_one_issuer() {

	bsdr_data_t bsdr_data;
	long b_amount = 0, s_amount = 0;

	const std::string issuer_name = "5850";
	Date st_date("2021-02-19");
	Date ed_date("2021-02-19");

	// get data
	bsdr_data = BSDR::get_data(st_date, ed_date, Market::ALL);
	
	// print
	for (const auto &bsdr_d: bsdr_data) {
		// OUTPUT(bsdr_d.first);
		std::cout << bsdr_d.first << ",";

		b_amount = s_amount = 0;
		for (const auto &bsdr: bsdr_d.second) {
			for (const auto &record: bsdr->records) {
				if (record->issuer_name == issuer_name) {
					b_amount += record->b_lot * record->px;
					s_amount += record->s_lot * record->px;
				}
			}
		}

		std::cout << b_amount << "," << s_amount << "," << b_amount - s_amount << std::endl;
	}
}

void parse_issuers() {

	// struct trade_detail {
	// 	int b_lot;
	// 	int s_lot;
	// 	long b_amount;
	// 	long s_amount;
	// };

	// std::map<std::string, std::map<std::string, struct trade_detail>> d;  // <K: issuer, V:<K: stock, V: detail>>

	// Date st_date("2021-02-17");
	// Date ed_date("2021-02-17");
	// bsdr_data_t bsdr_data = BSDR::get_data(st_date, ed_date, Market::ALL);

	// for (const auto &bsdr_d: bsdr_data) {
	// 	// std::cout << bsdr_d.first << std::endl;
	// 	for (const auto &bsdr: bsdr_d.second) {
	// 		for (const auto &record: bsdr->records) {

	// 			d[record->issuer_name][bsdr->stock_fc].b_lot += record->b_lot;
	// 			d[record->issuer_name][bsdr->stock_fc].s_lot += record->s_lot;
	// 			d[record->issuer_name][bsdr->stock_fc].b_amount += record->b_lot * record->px;
	// 			d[record->issuer_name][bsdr->stock_fc].s_amount += record->s_lot * record->px;

	// 			// if (record.issuer_name.substr(0,3) == "585") {

	// 			// }
	// 		}
	// 	}
	// }

	Date one_date("2021-02-18");
	bsdr_date_issuer_stock_t d = BSDR::get_analysis_data_date_issuer_stock(one_date, one_date, Market::ALL);

	// output
	std::ofstream my_file("output/output");

	// 分點明細
	// for (const auto &date_d: d) {
	// 	for (const auto & issuer_d: date_d.second) {
	// 		if (issuer_d.first == "9800") {
	// 			for (const auto &stock_d: issuer_d.second) {
	// 				output_delimiter_str(my_file, ",", { std::string("=\"" + stock_d.first + "\""), std::to_string(stock_d.second.b_amount/1000), std::to_string(stock_d.second.s_amount/1000), std::to_string((stock_d.second.b_amount - stock_d.second.s_amount)/1000) });
	// 			}
	// 		}
	// 	}
	// }

	// 券商分點進出金額排行
	long b_amount_sum, s_amount_sum;
	for (const auto &issuer_d: d["2021-02-18"]) {
		my_file << "=\"" << issuer_d.first << "\",";
		b_amount_sum = s_amount_sum = 0;
		for (const auto &stock_d: issuer_d.second) {
			b_amount_sum += stock_d.second.b_amount;
			s_amount_sum += stock_d.second.s_amount;
		}
		my_file << b_amount_sum << "," << s_amount_sum << "," << b_amount_sum - s_amount_sum << std::endl;
	}

	my_file.close();
}

void parse_debug(bsdr_data_t *bsdr_data) {
	std::ofstream my_file("output");
	for (const auto &bsdr_d: *bsdr_data) {
		for (const auto &bsdr: bsdr_d.second) {
			// my_file << bsdr->stock_fc << std::endl;

			for (const auto &record: bsdr->records) {
				if (bsdr->stock_fc == "4966" && record->issuer_name == "9200") {
					my_file << *record << std::endl;
				}
			}
		}
	}
	my_file.close();
}


