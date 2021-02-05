#include <iostream>

#include "BSDR/BSDR.h"
#include "util/util.h"

void parse_one_issuer(bsdr_data_t*);

int main(int argc, char *argv[]) {

	Date *st_date, *ed_date;
	bsdr_data_t *bsdr_data;

	st_date = new Date("2021-02-02");
	ed_date = new Date("2021-02-02");

	bsdr_data = new bsdr_data_t;
	BSDR::get_data(bsdr_data, st_date, ed_date, Market::ALL);

	parse_one_issuer(bsdr_data);


	return 0;
}

void parse_one_issuer(bsdr_data_t *bsdr_data) {

	struct trade_detail {
		int b_lot;
		int s_lot;
		long b_amount;
		long s_amount;
	};

	std::map<std::string, struct trade_detail> d;

	for (const auto &bsdr_d: *bsdr_data) {
		// std::cout << bsdr_d.first << std::endl;
		for (const auto &bsdr: bsdr_d.second) {
			for (const auto &record: bsdr->records) {

				d[record->issuer_name].b_lot += record->b_lot;
				d[record->issuer_name].s_lot += record->s_lot;
				d[record->issuer_name].b_amount += record->b_lot * record->px;
				d[record->issuer_name].s_amount += record->s_lot * record->px;

				// if (record.issuer_name.substr(0,3) == "585") {

				// }
			}
		}
	}

	// for (const auto &issuer_d: d) {
	// 	std::cout << issuer_d.first << ": " << issuer_d.second.b_amount << " " << issuer_d.second.s_amount << std::endl;
	// }
}




