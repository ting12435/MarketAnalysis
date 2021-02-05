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

}