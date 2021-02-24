#include "BSDR.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>


#define BSDR_TSE_FOLDER "../../data/BuySellDailyReport/TSE/"
#define BSDR_OTC_FOLDER "../../data/BuySellDailyReport/OTC/"

static std::string bsdr_trim(const std::string& s) {
	std::string result_s;
	for (const auto &c: s) {
		if (c != '"' && c != ',')
			result_s += c;
	}
	return result_s;
}

static void split_otc(const std::string& s, std::vector<std::string>& sv) {
	// "1","9100  ¸s¯q","1.26","68,000","0",,"2","9A00  ¥ÃÂ×ª÷","1.26","0","68,000"

	sv.clear();

	bool entry_start = false;
	std::string entry_str = "";
	for (const auto &c: s) {

		if (c == '"')
			entry_start = !entry_start;

		if (!entry_start) {

			if (c == ',') {
				sv.emplace_back(entry_str);
				entry_str = "";
			}

		} else if (c != '"') {
			entry_str += c;
		}
	}

	sv.emplace_back(entry_str);
}

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

#if __cplusplus < 201703L
BSDR* read_file(std::string filename, Market market) {
	BSDR *bsdr_ptr = NULL;

	return bsdr_ptr;
}
#else  // c++17
BSDR* read_file(fs::directory_entry file, Market market) {

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
	if (market == Market::TSE)
		split(fn, sv);
	else if (market == Market::OTC)
		split(fn, sv, "_");
	else {
		std::cerr << "error market" << std::endl;;
		exit(-1);
	}
	bsdr_ptr->stock_fc = sv[0];
	// OUTPUT(bsdr_ptr->stock_fc);

	// read
	// std::cout << "read " << file.path().filename() << std::endl;
	while (!ifs.eof()) {
		ifs.getline(line_c, sizeof(line_c));
		line_str = line_c;
		if (!line_str.empty()) {
			// std::cout << line_str << std::endl;

			if (market == Market::TSE)
				split(line_str, sv);
			else if (market == Market::OTC)
				split_otc(line_str, sv);
			else {
				std::cerr << "throw error" << std::endl;;
				OUTPUT("error market");
				OUTPUT(fn);
				OUTPUT(line_str);
				exit(-1);
			}

			// std::cout << "[" << sv[1] << "] [" << only_number_and_str(sv[1]) << "]" << std::endl;
			// int i = 0;
			// for (const auto &token: sv) {
			// 	std::cout << i++ << ":" << only_number_and_str(token) << " " << std::endl;
			// }

			try {

				// debug
				// if (fn == "4966_1100217") {
				// // if (market == Market::OTC) {
				// 	std::cout << "line_str = [" << line_str << "]" << std::endl;
				// 	std::cout << "sv.size() = " << sv.size() << std::endl;
				// 	for (const auto &t: sv) {
				// 		std::cout << "{" << t << "} " << std::endl;
				// 	}
				// 	// output_delimiter_str(std::cout, ": ", { "0", bsdr_trim(sv[0]) });
				// 	// output_delimiter_str(std::cout, ": ", { "1", bsdr_trim(sv[1]) });
				// 	// output_delimiter_str(std::cout, ": ", { "2", bsdr_trim(sv[2]) });
				// 	// output_delimiter_str(std::cout, ": ", { "3", bsdr_trim(sv[3]) });
				// 	// output_delimiter_str(std::cout, ": ", { "4", bsdr_trim(sv[4]) });
				// 	// output_delimiter_str(std::cout, ": ", { "5", bsdr_trim(sv[5]) });
				// 	// output_delimiter_str(std::cout, ": ", { "6", bsdr_trim(sv[6]) });
				// 	exit(-1);
				// }

				record_ptr = new BSDR_record();
				record_ptr->seq = std::stoi(only_number_and_str(sv[0]));
				// record_ptr->issuer_name = only_number_and_str(sv[1]);
				record_ptr->issuer_name = sv[1].substr(0, 4);
				record_ptr->px = std::stof(bsdr_trim(sv[2]));
				record_ptr->b_lot = std::stoi(bsdr_trim(sv[3]));
				record_ptr->s_lot = std::stoi(bsdr_trim(sv[4]));

				// if (bsdr_ptr->stock_fc == "4966" && record_ptr->issuer_name == "9200") {
				// 	std::cout << "line_str = [" << line_str << "]" << std::endl;
				// 	std::cout << "sv.size() = " << sv.size() << std::endl;
				// 	for (const auto &t: sv)
				// 		std::cout << "{" << t << "} " << std::endl;
				// 	std::cout << *record_ptr << std::endl;
					
				// }

				bsdr_ptr->records.emplace_back(record_ptr);

				if ((market == Market::OTC && sv.size() > 5) || (market == Market::TSE && sv[6] != "")) {
					record_ptr = new BSDR_record();
					record_ptr->seq = std::stoi(only_number_and_str(sv[6]));
					// record_ptr->issuer_name = only_number_and_str(sv[7]);
					record_ptr->issuer_name = sv[7].substr(0, 4);
					record_ptr->px = std::stof(bsdr_trim(sv[8]));
					record_ptr->b_lot = std::stoi(bsdr_trim(sv[9]));
					record_ptr->s_lot = std::stoi(bsdr_trim(sv[10]));

					// if (bsdr_ptr->stock_fc == "2330" && record_ptr->issuer_name == "9200")
					// 	std::cout << *record_ptr << std::endl;

					bsdr_ptr->records.emplace_back(record_ptr);
				}

			} catch(...) {
				std::cerr << "throw error" << std::endl;;
				std::cout << "Market=" << market << std::endl;
				OUTPUT(fn);
				OUTPUT(line_str);
				exit(-1);
			}
		}
	}

	close_file:
	ifs.close();

	return bsdr_ptr;
}
#endif

#if __cplusplus < 201703L
bsdr_data_t BSDR::get_data(Date st_date, Date ed_date, Market market) {
	bsdr_data_t d;

	return d;
}
#else  // c++17
bsdr_data_t BSDR::get_data(Date st_date, Date ed_date, Market market) {
	bsdr_data_t d;
	BSDR *bsdr;
	std::vector<fs::path> dirs;
	Market current_marekt;

	Date current_date(st_date.date_str);
	while (current_date <= ed_date) {

		// dir
		dirs.clear();
		if (market == Market::TSE || market == Market::ALL)
			dirs.emplace_back(fs::path{BSDR_TSE_FOLDER + current_date.date_str});
		if (market == Market::OTC || market == Market::ALL)
			dirs.emplace_back(fs::path{BSDR_OTC_FOLDER + current_date.date_str});

		// const fs::path dir{BSDR_TSE_FOLDER + current_date.date_str};

		for (const auto &dir: dirs) {

			if (fs::exists(dir)) {

				// OUTPUT(dir);
				std::cout << " -- reading dir:" << dir << std::endl;

				if (dir.string().find(BSDR_TSE_FOLDER) != std::string::npos)
					current_marekt = Market::TSE;
				else if (dir.string().find(BSDR_OTC_FOLDER) != std::string::npos)
					current_marekt = Market::OTC;

				// OUTPUT(current_marekt);

				for (const auto &file: fs::directory_iterator(dir)) {

					std::string file_ex = file.path().extension().string();
					// std::transform(file_ex.begin(), file_ex.end(), file_ex.begin(), std::tolower);

					if (file_ex == ".csv" || file_ex == ".CSV") {

						bsdr = read_file(file, current_marekt);

						if (bsdr != NULL) {
							// OUTPUT(*bsdr);
							d[current_date.date_str].emplace_back(bsdr);
						}
					}
				}
			}
		}

		current_date.add(1);
	}

	return d;
}
#endif

bsdr_date_issuer_stock_t BSDR::get_analysis_data_date_issuer_stock(Date st_date, Date ed_date, Market market) {
	bsdr_data_t bsdr_data = BSDR::get_data(st_date, ed_date, market);
	bsdr_date_issuer_stock_t d;

	for (const auto &bsdr_d: bsdr_data) {
		for (const auto &bsdr: bsdr_d.second) {
			for (const auto &record: bsdr->records) {
				d[bsdr_d.first][record->issuer_name][bsdr->stock_fc].b_lot += record->b_lot;
				d[bsdr_d.first][record->issuer_name][bsdr->stock_fc].s_lot += record->s_lot;
				d[bsdr_d.first][record->issuer_name][bsdr->stock_fc].b_amount += record->b_lot * record->px;
				d[bsdr_d.first][record->issuer_name][bsdr->stock_fc].s_amount += record->s_lot * record->px;
			}
		}
	}

	return d;
}

bsdr_date_stock_issuer_t BSDR::get_analysis_data_date_stock_issuer(Date st_date, Date ed_date, Market market) {
	bsdr_data_t bsdr_data = BSDR::get_data(st_date, ed_date, market);
	bsdr_date_stock_issuer_t d;

	for (const auto &bsdr_d: bsdr_data) {
		for (const auto &bsdr: bsdr_d.second) {
			for (const auto &record: bsdr->records) {
				d[bsdr_d.first][bsdr->stock_fc][record->issuer_name].b_lot += record->b_lot;
				d[bsdr_d.first][bsdr->stock_fc][record->issuer_name].s_lot += record->s_lot;
				d[bsdr_d.first][bsdr->stock_fc][record->issuer_name].b_amount += record->b_lot * record->px;
				d[bsdr_d.first][bsdr->stock_fc][record->issuer_name].s_amount += record->s_lot * record->px;
			}
		}
	}

	return d;
}
bsdr_issuer_date_stock_t BSDR::get_analysis_data_issuer_date_stock(Date st_date, Date ed_date, Market market) {
	bsdr_data_t bsdr_data = BSDR::get_data(st_date, ed_date, market);
	bsdr_issuer_date_stock_t d;

	for (const auto &bsdr_d: bsdr_data) {
		for (const auto &bsdr: bsdr_d.second) {
			for (const auto &record: bsdr->records) {
				d[record->issuer_name][bsdr_d.first][bsdr->stock_fc].b_lot += record->b_lot;
				d[record->issuer_name][bsdr_d.first][bsdr->stock_fc].s_lot += record->s_lot;
				d[record->issuer_name][bsdr_d.first][bsdr->stock_fc].b_amount += record->b_lot * record->px;
				d[record->issuer_name][bsdr_d.first][bsdr->stock_fc].s_amount += record->s_lot * record->px;
			}
		}
	}

	return d;
}

bsdr_issuer_stock_date_t BSDR::get_analysis_data_issuer_stock_date(Date st_date, Date ed_date, Market market) {
	bsdr_data_t bsdr_data = BSDR::get_data(st_date, ed_date, market);
	bsdr_issuer_stock_date_t d;

	for (const auto &bsdr_d: bsdr_data) {
		for (const auto &bsdr: bsdr_d.second) {
			for (const auto &record: bsdr->records) {
				d[record->issuer_name][bsdr->stock_fc][bsdr_d.first].b_lot += record->b_lot;
				d[record->issuer_name][bsdr->stock_fc][bsdr_d.first].s_lot += record->s_lot;
				d[record->issuer_name][bsdr->stock_fc][bsdr_d.first].b_amount += record->b_lot * record->px;
				d[record->issuer_name][bsdr->stock_fc][bsdr_d.first].s_amount += record->s_lot * record->px;
			}
		}
	}

	return d;
}

bsdr_stock_issuer_date_t BSDR::get_analysis_data_stock_issuer_date(Date st_date, Date ed_date, Market market) {
	bsdr_data_t bsdr_data = BSDR::get_data(st_date, ed_date, market);
	bsdr_stock_issuer_date_t d;

	for (const auto &bsdr_d: bsdr_data) {
		for (const auto &bsdr: bsdr_d.second) {
			for (const auto &record: bsdr->records) {
				d[bsdr->stock_fc][record->issuer_name][bsdr_d.first].b_lot += record->b_lot;
				d[bsdr->stock_fc][record->issuer_name][bsdr_d.first].s_lot += record->s_lot;
				d[bsdr->stock_fc][record->issuer_name][bsdr_d.first].b_amount += record->b_lot * record->px;
				d[bsdr->stock_fc][record->issuer_name][bsdr_d.first].s_amount += record->s_lot * record->px;
			}
		}
	}

	return d;
}

bsdr_stock_date_issuer_t BSDR::get_analysis_data_stock_date_issuer(Date st_date, Date ed_date, Market market) {
	bsdr_data_t bsdr_data = BSDR::get_data(st_date, ed_date, market);
	bsdr_stock_date_issuer_t d;

	for (const auto &bsdr_d: bsdr_data) {
		for (const auto &bsdr: bsdr_d.second) {
			for (const auto &record: bsdr->records) {
				d[bsdr->stock_fc][bsdr_d.first][record->issuer_name].b_lot += record->b_lot;
				d[bsdr->stock_fc][bsdr_d.first][record->issuer_name].s_lot += record->s_lot;
				d[bsdr->stock_fc][bsdr_d.first][record->issuer_name].b_amount += record->b_lot * record->px;
				d[bsdr->stock_fc][bsdr_d.first][record->issuer_name].s_amount += record->s_lot * record->px;
			}
		}
	}

	return d;
}


void BSDR::tester() {
	// // const std::string s = "\"1234\",\"111\",\"\",\"222\",,\"333\"";
	// // const std::string s = "\"1\",\"9A00  ¥ÃÂ×ª÷\",\"1.10\",\"16,000\",\"16,000\"";
	// // const std::string s = "\"1\",\"9100  ¸s¯q\",\"1.26\",\"68,000\",\"0\",,\"2\",\"9A00  ¥ÃÂ×ª÷\",\"1.26\",\"0\",\"68,000\"";
	// // const std::string s = "\"1\",\"7790  ?겼\",\"0.60\",\"25,000\",\"0\",,\"2\",\"9647  ?I???ثH\",\"0.60\",\"0\",\"25,000\"";
	// const std::string s = "\"585\",\"9200  ³Í°ò\",\"1,360.00\",\"50\",\"0\",,\"586\",\"9200  ³Í°ò\",\"1,370.00\",\"2,000\",\"0\"";
	// // const std::string s = "1,9A00¥ÃÂ×ª÷,0.89,21000,21000,,,,,, ";
	// // const std::string s = "4197,9200³Í°ò,631.00,1000,0,,4198,9200³Í°ò,632.00,92000,0 ";
	// std::vector<std::string> sv;

	// split_otc(s, sv);
	// // split(s, sv);

	// for (const auto &t: sv) {
	// 	OUTPUT(t);
	// }

	std::vector<std::string> v = get_files_in_dir("/home/tim/");
	for (const auto &fn: v) {
		OUTPUT(fn);
	}
}




