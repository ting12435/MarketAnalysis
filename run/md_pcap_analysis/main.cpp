#include <map>
#include <iterator>
#include <cassert>
#include <getopt.h>

#include "md_pcap/md_pcap.h"
#include "util/util.h"

#define PCAP_FOLDER "/data/database/2in1/tcpdump/"
// #define PCAP_FOLDER "/data/tim/"

void uplimit();
void large_amount();
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
	else if (g_var.type == "large_amount")
		large_amount();
	else if (g_var.type == "debug")
		debug();

	return 0;

	usage_error:
	fprintf(stderr, "Usage: %s\n", argv[0]);
	fprintf(stderr, "%9s [--type] [--d1] [--d2]\n", " ");
	fprintf(stderr, "  --type: [uplimit] [large_amount] [debug]\n");
	fprintf(stderr, "\ne.g.\n");
	fprintf(stderr, "taskset -c 5 %s --type debug\n", argv[0]);
	fprintf(stderr, "taskset -c 5 %s --type large_amount --d1 2021-04-26 --d2 2021-04-28\n", argv[0]);
	fprintf(stderr, "taskset -c 5 %s --type uplimit --d1 2021-04-23 --d2 2021-04-26\n", argv[0]);
	return EXIT_FAILURE;
}

void uplimit() {

	struct info {
		int uplimit_px;
		int first_px;
		int highest_px;
		// bool last_match_mode;
		bool uplimit_flag;
		bool after_090000;
	};

	std::map<Date, std::map<std::string, struct info>> m;
	std::map<Date, std::map<std::string, struct info>>::iterator cur_iter, prv_iter;
	struct md *frame;
	MD md;
	int _px;
	struct info *info_ptr;
	char buf[1000];

	Date current_date(g_var.d1->date_str);
	while (current_date <= *(g_var.d2)) {

		OneDayPcap one_day_pcap(current_date);
		if (!one_day_pcap) {
			std::cout << "error: " << one_day_pcap.get_error();
		} else {

			m.emplace(current_date, std::map<std::string, struct info>());
			
			// while ((frame = one_day_pcap.get_pcap_record_data()) != nullptr) {
			while ((frame = one_day_pcap.get_md()) != nullptr) {

				if (frame->esc_code != 27)
					continue;

				md.set_data(frame);
				
				if (md.is_md) {					

					if (md.fmt_code == 0x6) {

						if (m[current_date].find(md.feedcode) == m[current_date].end())
							m[current_date].emplace(md.feedcode, info{});

						info_ptr = &m[current_date][md.feedcode];
					
						// up limit
						if (!info_ptr->uplimit_flag) {
							if (md.match_time_sec >= 90000) {
								// if (md.trade_limit == 0x2) {  // 漲停成交
								if (md.b_limit == 0x2) {  // 漲停買進

									_px = md.bid_px[0] != 0 ? md.bid_px[0] : md.bid_px[1];
									info_ptr->uplimit_px = _px;
									info_ptr->uplimit_flag = true;
								}
							}
						}

						// first px after 09:00
						// if (!m[current_date][md.feedcode].last_match_mode && md.match_mode) {
						if (!info_ptr->after_090000 && md.match_time_sec >= 90000) {
							_px = md.bid_px[0] != 0 ? md.bid_px[0] : md.bid_px[1];
							info_ptr->first_px = _px;
							// m[current_date][md.feedcode].last_match_mode = true;
							info_ptr->after_090000 = true;
						}

						// highest_px
						if (md.match_time_sec >= 90000) {
							if (md.trade_px >= info_ptr->highest_px)
								info_ptr->highest_px = md.trade_px;
						}
					}
				}

			}
			// std::cout << "error: " << one_day_pcap.get_error() << std::endl;
		}

		current_date.add(1);
	}

	std::cout << "--------------------" << std::endl;

	//
	// std::cout << "m.size()=" << m.size() << std::endl;
	// for (const auto &date_d: m) {
	// 	std::cout << date_d.first << " date_d.second.size()=" << date_d.second.size() << std::endl;
	// }

	// std::cout << "--------------------" << std::endl;

	//
	std::cout << "m.size()=" << m.size() << std::endl;
	for (const auto &date_d: m) {
		cur_iter = m.find(date_d.first);
		prv_iter = std::prev(cur_iter);
		std::cout << date_d.first << " " << (prv_iter == cur_iter) << " " << (prv_iter == m.begin()) << " " << (prv_iter == m.end()) << std::endl;
	}

	// analysis
	int fraction, denominator;
	// std::cout << "m.size()=" << m.size() << std::endl;
	for (const auto &date_d: m) {
		cur_iter = m.find(date_d.first);
		prv_iter = std::prev(cur_iter);

		// std::cout << date_d.first << " cur_iter->second.size()=" << cur_iter->second.size() << std::endl;

		if ((m.size() == 2 && prv_iter != cur_iter) || 
			(m.size() > 2 && prv_iter != m.end())) {

			fraction = denominator = 0;

			for (const auto &stock_d: date_d.second) {
				if (prv_iter->second.find(stock_d.first) != prv_iter->second.end() && 
					cur_iter->second.find(stock_d.first) != cur_iter->second.end()) {
					auto px1 = prv_iter->second[stock_d.first].uplimit_px;
					// auto px2 = cur_iter->second[stock_d.first].first_px;
					auto px2 = cur_iter->second[stock_d.first].highest_px;
					if (px1 > 0) {
						denominator++;
						// std::cout << prv_iter->first << " " << cur_iter->first << " " << stock_d.first << " " << px1 << " " << px2 << " " << (px2 >= px1) << std::endl;
						if (px2 >= px1) {
							fraction++;
						}
					}
				}
			}

			snprintf(buf, sizeof(buf), "%4d %4d %.2f\n", fraction, denominator, (double)fraction/denominator);
			std::cout << prv_iter->first << "-" << cur_iter->first << " "  << buf << std::endl;
		}
	}
}

void large_amount() {

	struct info {
		int accm_trade_lot;
	};

	std::map<Date, std::map<std::string, struct info>> m;

	struct md *frame;
	MD md;
	struct info *info_ptr;

	Date current_date(g_var.d1->date_str);
	while (current_date <= *(g_var.d2)) {
		OneDayPcap one_day_pcap(current_date);
		if (!one_day_pcap) {
			std::cout << "error: " << one_day_pcap.get_error();
		} else {
			// while ((frame = one_day_pcap.get_pcap_record_data()) != nullptr) {
			while ((frame = one_day_pcap.get_md()) != nullptr) {

				if (frame->esc_code != 27)
					continue;

				md.set_data(frame);

				if (md.fmt_code == 0x6) {

					if (m[current_date].find(md.feedcode) == m[current_date].end())
						m[current_date].emplace(md.feedcode, info{});

					info_ptr = &m[current_date][md.feedcode];

					if (!md.is_est && md.trade_lt != -1)
						info_ptr->accm_trade_lot += md.trade_lt;

					if (md.feedcode == "2330  ") {
						std::cout << md.trade_lt << " " << md.accm_trade_lot << " " << info_ptr->accm_trade_lot << std::endl;

						if (md.match_time_sec > 90000)
							print_hexdump((char*)frame, md.md_len);
					}

				}
			}
		}

		current_date.add(1);
	}

	std::cout << "--------------------" << std::endl;

	// analysis
	for (const auto &date_d: m) {
		for (const auto &stock_d: date_d.second) {

			if (stock_d.first == "2330  ")
				std::cout << date_d.first << " " << stock_d.first << " " << stock_d.second.accm_trade_lot << std::endl;

		}
	}
}

void debug() {
	struct md *frame;
	MD md;
	int last_seq = 0;

	Date current_date("2021-04-28");
	OneDayPcap one_day_pcap(current_date);
	if (!one_day_pcap) {
		std::cout << "error: " << one_day_pcap.get_error() << std::endl;
	} else {
		// while ((frame = one_day_pcap.get_pcap_record_data()) != nullptr) {
		while ((frame = one_day_pcap.get_md()) != nullptr) {

			if (frame->esc_code != 27)
					continue;

			md.set_data(frame);

			if (md.is_md && md.fmt_code == 0x6) {

				// if (md.feedcode == "2009  ") {
				// 	md.print_detail();
				// 	// if (md.is_open) {
				// 		// print_hexdump((char*)frame, md.md_len);
				// 	// }
				// 	if (md.match_time_sec > 90000)
				// 		exit(-1);
				// }

				std::cout << md.seq << std::endl;
				if (md.seq != last_seq + 1) {
					std::cerr << "miss " << last_seq << " " << md.seq << std::endl;
					exit(-1);
				}
				last_seq = md.seq;


			}
		}
	}
}

