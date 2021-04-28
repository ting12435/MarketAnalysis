#include <map>
#include <iterator>
#include <cassert>
#include <getopt.h>

#include "md_pcap/md_pcap.h"
#include "util/util.h"

// #define PCAP_FOLDER "/data/database/2in1/tcpdump/"
#define PCAP_FOLDER "/data/tim/"

void uplimit();
void debug();

struct var {
	std::string type;
	Date *d1;
	Date *d2;
} g_var;

void init_g_var() {
	g_var.type = "";
	g_var.d1 = nullptr;
	g_var.d2 = nullptr;
}

bool check_g_var() {
	if (g_var.d1 == nullptr && g_var.d2 == nullptr)
		return false;
	else if (g_var.d1 == nullptr)
		g_var.d1 = g_var.d2;
	else if (g_var.d2 == nullptr)
		g_var.d2 = g_var.d1;
	return true;
}

static struct option long_options[] = {
	{"type", required_argument, NULL, 0},
	{"d1", required_argument, NULL, 1},
	{"d2", required_argument, NULL, 2}
};

int main(int argc, char *argv[]) {

	int c;

	init_g_var();

	if (argc < 2) goto usage_error;

	while ((c = getopt_long (argc, argv, "", long_options, NULL)) != -1) {
		switch (c) {
			case 0:
				g_var.type = optarg;
				break;
			case 1:
				g_var.d1 = new Date(optarg);
				break;
			case 2:
				g_var.d2 = new Date(optarg);
				break;
			default:
				goto usage_error;
		}
	}

	// assert(check_g_var());

	pcap_folder = PCAP_FOLDER;
	pcap_market = "TSE";

	if (g_var.type == "uplimit")
		uplimit();
	else if (g_var.type == "debug")
		debug();

	return 0;

	usage_error:
	fprintf(stderr, "Usage: %s\n", argv[0]);
	fprintf(stderr, "%9s [--type] [--d1] [--d2]\n", " ");
	fprintf(stderr, "  --type: [uplimit] [debug]\n");
	fprintf(stderr, "\ne.g.\n");
	fprintf(stderr, "taskset -c 5 %s --type debug\n", argv[0]);
	fprintf(stderr, "taskset -c 5 %s --type uplimit --d1 2021-04-23 --d2 2021-04-26\n", argv[0]);
	return EXIT_FAILURE;
}

void uplimit() {

	struct info {
		int uplimit_px;
		int first_px;
		bool last_match_mode;
	};

	std::map<Date, std::map<std::string, struct info>> m;
	std::map<Date, std::map<std::string, struct info>>::iterator cur_iter, prv_iter;
	struct md *frame;
	MD md;
	int _px;

	Date current_date(g_var.d1->date_str);
	while (current_date <= *(g_var.d2)) {

		OneDayPcap one_day_pcap(current_date);
		if (!one_day_pcap) {
			std::cout << "error: " << one_day_pcap.get_error() << std::endl;
		} else {

			m.emplace(current_date, std::map<std::string, struct info>());
			
			while ((frame = one_day_pcap.get_pcap_record_data()) != nullptr) {

				md.set_data(frame);
				
				if (md.is_md) {

					if (m[current_date].find(md.feedcode) == m[current_date].end())
						m[current_date].emplace(md.feedcode, {});

					if (md.fmt_code == 0x6) {
					
						// 漲停成交
						if (md.trade_limit == 0x2) {

							// if (m[current_date].find(md.feedcode) == m[current_date].end()) {

								// md.print_detail();
								// print_hexdump((char*)frame, md.md_len);

								_px = md.bid_px[0] != 0 ? md.bid_px[0] : md.bid_px[1];

								m[current_date][md.feedcode].uplimit_px = _px;

							// }
						}

						// first px after 09:00
						if (!m[current_date][md.feedcode].last_match_mode && md.match_mode) {
							_px = md.bid_px[0] != 0 ? md.bid_px[0] : md.bid_px[1];
							m[current_date][md.feedcode].first_px = _px;
							m[current_date][md.feedcode].last_match_mode = true;
						}

						// if (md.is_open) {  // 開盤註記
							
							// if (prv_iter != cur_iter) {
							// 	// std::cout << "prv_iter " << prv_iter->first << std::endl;
							// 	if (prv_iter->second.find(md.feedcode) != prv_iter->second.end()) {

							// 		// if (md.feedcode == "1474  ") {
							// 		// 	// print_hexdump((char*)frame, md.md_len);
							// 		// 	md.print_detail();
							// 		// }

							// 		if (!m[current_date][md.feedcode].last_match_mode && md.match_mode) {

							// 			auto px = md.bid_px[0] != 0 ? md.bid_px[0] : md.bid_px[1];

							// 			if (px >= prv_iter->second[md.feedcode].uplimit_px) {
							// 				m[current_date][md.feedcode].open_higher_last_limit = true;
							// 			}
							// 			m[current_date][md.feedcode].open_px = px;
							// 			m[current_date][md.feedcode].last_match_mode = true;
							// 		}
							// 	}
							// }
						// }
					}
				}

			}
			// std::cout << "error: " << one_day_pcap.get_error() << std::endl;
		}

		current_date.add(1);
	}

	// analysis
	for (const auto &date_d: m) {

		cur_iter = m.find(date_d.first);
		prv_iter = std::prev(cur_iter);

		for (const auto &stock_d: date_d.second) {

		}
	}

	// output
	for (const auto &date_d: m) {
		for (const auto &stock_d: date_d.second) {
			std::cout << date_d.first << " " << stock_d.first << \
					" uplimit_px=" << stock_d.second.uplimit_px << \
					" open_higher_last_limit=" << stock_d.second.open_higher_last_limit << \
					" open_px=" << stock_d.second.open_px << std::endl;
		}
	}

}

void debug() {
	struct md *frame;
	MD md;

	Date current_date("2021-04-26");
	OneDayPcap one_day_pcap(current_date);
	if (!one_day_pcap) {
		std::cout << "error: " << one_day_pcap.get_error() << std::endl;
	} else {
		while ((frame = one_day_pcap.get_pcap_record_data()) != nullptr) {
			md.set_data(frame);
			if (md.is_md && md.fmt_code == 0x6) {
				if (md.feedcode == "1474  ") {
					md.print_detail();
					// if (md.is_open) {
						// print_hexdump((char*)frame, md.md_len);
					// }
					if (md.match_time_sec > 90000)
						exit(-1);
				}
			}
		}
	}
}

