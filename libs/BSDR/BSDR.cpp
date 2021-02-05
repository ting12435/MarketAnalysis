#include "BSDR.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#define BSDR_TSE_FOLDER "../BuySellDailyReport/TSE/"
#define BSDR_OTC_FOLDER "../BuySellDailyReport/OTC/"

std::ostream& operator<<(std::ostream& os, const BSDR_record& record) {
	os << record.seq << " " << record.issuer_name << " " << record.px << " " << record.b_lot << " " << record.s_lot;
	return os;
}

BSDR::BSDR() {
	
}

std::ostream& operator<<(std::ostream& os, const BSDR& bsdr) {
	os << bsdr.filename << " trade_date:" << bsdr.trade_date << " stock_fc:" << bsdr.stock_fc << " record_count:" << bsdr.records.size();
	return os;
}

BSDR* BSDR::read_file(fs::directory_entry file) {

	// std::cout << file.path() << std::endl;
	// std::cout << file.path().stem().string() << std::endl;
	// std::cout << file.path().extension().string() << std::endl;

	#define LINE_MAX_LEN 2000
	char line_c[LINE_MAX_LEN];
	std::string line_str;
	std::ifstream ifs;
	std::vector<std::string> sv;

	BSDR *bsdr_ptr = NULL;
	BSDR_record *record_ptr;

	std::string fn = file.path().stem().string();

	// filter
	// if (file.path().stem().string().size() != 4) return NULL;
	if (fn.find("(1)") != std::string::npos) return NULL;

	// OUTPUT(fn);
	ifs.open(file.path().string(), std::ios::in);

	if(!ifs) {
		// throw std::runtime_error(file.path().string() + ": " + std::strerror(errno));
		return NULL;
	}

	// first line
	ifs.getline(line_c, sizeof(line_c));
	line_str = line_c;
	if (line_str.empty() || line_str.find("Empty") != std::string::npos)
		goto close_file;

	// second, thrid line
	ifs.getline(line_c, sizeof(line_c));
	ifs.getline(line_c, sizeof(line_c));

	// file info
	bsdr_ptr = new BSDR;
	bsdr_ptr->filename = file.path().filename();
	split(fn, sv);
	bsdr_ptr->stock_fc = sv[0];
	OUTPUT(bsdr_ptr->stock_fc);

	// read
	// std::cout << "read " << file.path().filename() << std::endl;
	while (!ifs.eof()) {
		ifs.getline(line_c, sizeof(line_c));
		line_str = line_c;
		if (!line_str.empty()) {
			// std::cout << line_str << std::endl;

			split(line_str, sv);
			// std::cout << "[" << sv[1] << "] [" << only_number_and_str(sv[1]) << "]" << std::endl;
			// int i = 0;
			// for (const auto &token: sv) {
			// 	std::cout << i++ << ":" << only_number_and_str(token) << " " << std::endl;
			// }

			record_ptr = new BSDR_record();
			record_ptr->seq = std::stoi(only_number_and_str(sv[0]));
			// record_ptr->issuer_name = only_number_and_str(sv[1]);
			record_ptr->issuer_name = sv[1].substr(0, 4);
			record_ptr->px = std::stof(sv[2]);
			record_ptr->b_lot = std::stoi(sv[3]);
			record_ptr->s_lot = std::stoi(sv[4]);

			// std::cout << *record_ptr << std::endl;

			bsdr_ptr->records.emplace_back(record_ptr);

			if (only_number_and_str(sv[6]) != "") {
				record_ptr = new BSDR_record();
				record_ptr->seq = std::stoi(only_number_and_str(sv[6]));
				// record_ptr->issuer_name = only_number_and_str(sv[7]);
				record_ptr->issuer_name = sv[7].substr(0, 4);
				record_ptr->px = std::stof(sv[8]);
				record_ptr->b_lot = std::stoi(sv[9]);
				record_ptr->s_lot = std::stoi(sv[10]);

				// std::cout << *record_ptr << std::endl;

				bsdr_ptr->records.emplace_back(record_ptr);
			}

			// exit(-1);
			// goto close_file;
		}
	}

	close_file:
	ifs.close();

	return bsdr_ptr;
}

 int BSDR::get_data(bsdr_data_t *d, Date *st_date, Date *ed_date, Market market) {
	BSDR *bsdr;
	std::vector<fs::path> dirs;

	Date current_date(st_date->date_str);
	while (current_date <= *ed_date) {

		if (market == Market::TSE || market == Market::ALL)
			dirs.emplace_back(fs::path{BSDR_TSE_FOLDER + current_date.date_str});
		if (market == Market::OTC || market == Market::ALL)
			dirs.emplace_back(fs::path{BSDR_OTC_FOLDER + current_date.date_str});

		// const fs::path dir{BSDR_TSE_FOLDER + current_date.date_str};

		for (const auto &dir: dirs) {

			if (fs::exists(dir)) {

				OUTPUT(dir);

				for (const auto &file: fs::directory_iterator(dir)) {

					std::string file_ex = file.path().extension().string();
					// std::transform(file_ex.begin(), file_ex.end(), file_ex.begin(), std::tolower);

					if (file_ex == ".csv" || file_ex == ".CSV") {

						bsdr = BSDR::read_file(file);

						if (bsdr != NULL) {
							// OUTPUT(*bsdr);

							(*d)[current_date.date_str].emplace_back(bsdr);
						}
					}
				}
			}
		}

		current_date.add(1);
	}

	return 1;
}




