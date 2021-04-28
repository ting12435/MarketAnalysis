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
		bool open_higher_last_limit;
		int open_px;
		bool last_match_mode;
	};

	std::map<Date, std::map<std::string, struct info>> m;  // K:data V:{K:stock V:uplimit_price}
	std::map<Date, std::map<std::string, struct info>>::iterator cur_iter, prv_iter;
	struct md *frame;
	MD md;

	Date current_date(g_var.d1->date_str);
	while (current_date <= *(g_var.d2)) {

		OneDayPcap one_day_pcap(current_date);
		if (!one_day_pcap) {
			std::cout << "error: " << one_day_pcap.get_error() << std::endl;
		} else {
			// m.emplace(std::make_pair(current_date, { "",  {}}));
			m.emplace(current_date, std::map<std::string, struct info>());
			cur_iter = m.find(current_date);
			prv_iter = std::prev(cur_iter);

			// std::cout << "cur_iter " << cur_iter->first << std::endl;
			// std::cout << "prv_iter " << prv_iter->first << std::endl;

			while ((frame = one_day_pcap.get_pcap_record_data()) != nullptr) {

				md.set_data(frame);
				
				if (md.is_md) {

					if (md.fmt_code == 0x6) {
					
						// md.print_detail();
						// print_hexdump((char*)frame, 500);
						// exit(-1);

						if (md.trade_limit == 0x2) {  // 漲停成交

							if (m[current_date].find(md.feedcode) == m[current_date].end()) {

								// md.print_detail();
								// print_hexdump((char*)frame, md.md_len);

								// m[current_date][md.feedcode] = md.trade_px;
								m[current_date][md.feedcode].uplimit_px = md.bid_px[0] != 0 ? md.bid_px[0] : md.bid_px[1];

							}
						}

						// if (md.is_open) {  // 開盤註記
							// if (md.feedcode == "1474  ") {
							// 	print_hexdump((char*)frame, md.md_len);
							// }
							if (prv_iter != cur_iter) {
								// std::cout << "prv_iter " << prv_iter->first << std::endl;
								if (prv_iter->second.find(md.feedcode) != prv_iter->second.end()) {
									if (!m[current_date][md.feedcode].last_match_mode && md.match_mode) {
										if (md.trade_px >= prv_iter->second[md.feedcode].uplimit_px) {
											m[current_date][md.feedcode].open_higher_last_limit = true;
										}
										m[current_date][md.feedcode].open_px = md.trade_px;
										m[current_date][md.feedcode].last_match_mode = true;
									}
								}
							}
						// }
					}
				}

			}
			// std::cout << "error: " << one_day_pcap.get_error() << std::endl;
		}

		current_date.add(1);
	}

	// output
	for (const auto &date_d: m) {
		for (const auto &stock_d: date_d.second) {
			std::cout << date_d.first << " " << stock_d.first << " " << stock_d.second.uplimit_px << " " << stock_d.second.open_higher_last_limit << " " << stock_d.second.open_px << std::endl;
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
				if (md.feedcode == "2330  ") {
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

