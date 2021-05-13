#include <iomanip>
#include <map>
#include <iterator>
#include <cassert>
#include <cstring>
#include <getopt.h>

#include "md_pcap/md_pcap.h"
#include "util/util.h"

// #define PCAP_FOLDER "/data/database/2in1/tcpdump/"
#define PCAP_FOLDER "/data/tim/"

void uplimit();
void large_amount();
void debug();
void debug_seq_check();
void interactive();

struct var {
	std::string type;
	Date *d1;
	Date *d2;
	std::string feedcode;
} g_var;

void init_g_var() {
	g_var.type = "";
	g_var.d1 = nullptr;
	g_var.d2 = nullptr;
	g_var.feedcode = "";
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
	{"d2", required_argument, NULL, 2},
	{"feedcode", required_argument, NULL, 3}
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
			case 3:
				g_var.feedcode = optarg;
				break;
			default:
				goto usage_error;
		}
	}

	// assert(check_g_var());
	check_g_var();

	pcap_folder = PCAP_FOLDER;
	pcap_market = "TSE";

	if (g_var.type == "uplimit")
		uplimit();
	else if (g_var.type == "large_amount")
		large_amount();
	else if (g_var.type == "debug")
		debug();
	else if (g_var.type == "debug_seq_check")
		debug_seq_check();
	else if (g_var.type == "interactive")
		interactive();

	return 0;

	usage_error:
	fprintf(stderr, "Usage: %s\n", argv[0]);
	fprintf(stderr, "%9s [--type] [--d1] [--d2] [--feedcode]\n", " ");
	fprintf(stderr, "  --type: [uplimit] [large_amount] [debug]\n");
	fprintf(stderr, "\ne.g.\n");
	fprintf(stderr, "taskset -c 5 %s --type debug\n", argv[0]);
	fprintf(stderr, "taskset -c 5 %s --type large_amount --d1 2021-05-04 --feedcode 4529\n", argv[0]);
	fprintf(stderr, "taskset -c 5 %s --type uplimit --d1 2021-04-23 --d2 2021-04-26\n", argv[0]);
	fprintf(stderr, "taskset -c 5 %s --type interactive --d1 2021-04-28 --feedcode 2603\n", argv[0]);
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
			std::cerr << "error: " << one_day_pcap.get_last_error() << std::endl;;
		} else {

			m.emplace(current_date, std::map<std::string, struct info>());
			
			// while ((frame = one_day_pcap.get_pcap_record_data()) != nullptr) {
			while (true) {

				if (!one_day_pcap.get_md(&frame)) {
					std::cerr << "error: " << one_day_pcap.get_last_error() << std::endl;
					break;
				}

				if (frame == nullptr)
					break;

				md.set_data(frame);
				
				if (md.is_md) {					

					if (md.fmt_code == 0x6) {

						if (m[current_date].find(md.feedcode) == m[current_date].end())
							m[current_date].emplace(md.feedcode, info{});

						info_ptr = &m[current_date][md.feedcode];
					
						// up limit
						if (!info_ptr->uplimit_flag) {
							// if (md.match_time_sec >= 90000) {
							// if (md.match_time_sec >= 110000) {
								if (md.trade_limit == 0x2 && !md.is_est) {  // 漲停成交
								// if (md.b_limit == 0x2) {  // 漲停買進

									// _px = md.bid_px[0] != 0 ? md.bid_px[0] : md.bid_px[1];
									_px = md.trade_px;
									info_ptr->uplimit_px = _px;
									info_ptr->uplimit_flag = true;
								}
							// }
						}

						// first px after 09:00
						// if (!m[current_date][md.feedcode].last_match_mode && md.match_mode) {
						// if (!info_ptr->after_090000 && md.match_time_sec >= 90000) {
						if (md.is_open) {
							_px = md.bid_px[0] != 0 ? md.bid_px[0] : md.bid_px[1];
							// _px = md.ask_px[0] != 0 ? md.ask_px[0] : md.ask_px[1];
							info_ptr->first_px = _px;
							// m[current_date][md.feedcode].last_match_mode = true;
							// info_ptr->after_090000 = true;
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
	// std::cout << "m.size()=" << m.size() << std::endl;
	// for (const auto &date_d: m) {
	// 	cur_iter = m.find(date_d.first);
	// 	prv_iter = std::prev(cur_iter);
	// 	std::cout << date_d.first << " " << (prv_iter == cur_iter) << " " << (prv_iter == m.begin()) << " " << (prv_iter == m.end()) << std::endl;
	// }

	// analysis
	// int fraction, denominator;
	// for (const auto &date_d: m) {
	// 	cur_iter = m.find(date_d.first);
	// 	prv_iter = std::prev(cur_iter);

	// 	// std::cout << date_d.first << " cur_iter->second.size()=" << cur_iter->second.size() << std::endl;

	// 	if ((m.size() == 2 && prv_iter != cur_iter) || 
	// 		(m.size() > 2 && prv_iter != m.end())) {

	// 		fraction = denominator = 0;

	// 		for (const auto &stock_d: date_d.second) {
	// 			if (prv_iter->second.find(stock_d.first) != prv_iter->second.end() && 
	// 				cur_iter->second.find(stock_d.first) != cur_iter->second.end()) {
	// 				auto px1 = prv_iter->second[stock_d.first].uplimit_px;
	// 				// auto px2 = cur_iter->second[stock_d.first].first_px;
	// 				auto px2 = cur_iter->second[stock_d.first].highest_px;
	// 				if (px1 > 0) {
	// 					denominator++;
	// 					std::cout << prv_iter->first << " " << cur_iter->first << " " << stock_d.first << " " << px1 << " " << px2 << " " << (px2 >= px1) << std::endl;
	// 					if (px2 >= px1) {
	// 						fraction++;
	// 					}
	// 				}
	// 			}
	// 		}

	// 		snprintf(buf, sizeof(buf), "%4d %4d %.2f", fraction, denominator, (double)fraction/denominator);
	// 		std::cout << prv_iter->first << "-" << cur_iter->first << " "  << buf << std::endl;
	// 	}
	// }

	for (const auto &date_d: m) {
		for (const auto &stock_d: date_d.second) {
			auto _info_ptr = &stock_d.second;
			if (_info_ptr->uplimit_flag) {
				std::cout << stock_d.first << std::endl;
			}
		}
	}
}

void large_amount() {

	struct trade_list {
		int match_time_sec;
		int bid_px;
		int ask_px;
		int trade_px;
		int trade_lt;
		int accm_trade_lot;
	};

	struct info {
		int accm_trade_lot;
		int trade_bid_cnt;
		int trade_bid_lot;
		int trade_ask_cnt;
		int trade_ask_lot;
		int trade_gap_cnt;
		int trade_gap_lot;
		int trade_oth_cnt;
		int trade_oth_lot;
		std::map<int, int> trade_bid_map;  // K:trade lot, V:count
		std::map<int, int> trade_ask_map;  // K:trade lot, V:count
		std::vector<struct trade_list> trade_list_vec;
		int trade_list_accm_trade_lot;
	};

	std::map<Date, std::map<std::string, struct info>> m;

	struct md *frame;
	MD md;
	struct info *info_ptr;

	Date current_date(g_var.d1->date_str);
	while (current_date <= *(g_var.d2)) {
		OneDayPcap one_day_pcap(current_date);
		if (!one_day_pcap) {
			std::cerr << "error: " << one_day_pcap.get_last_error() << std::endl;;
		} else {
			// while ((frame = one_day_pcap.get_pcap_record_data()) != nullptr) {
			while (true) {

				if (!one_day_pcap.get_md(&frame)) {
					std::cerr << "error: " << one_day_pcap.get_last_error() << std::endl;
					break;
				}

				if (frame == nullptr)
					break;

				md.set_data(frame);

				if (md.fmt_code == 0x6) {

					if (m[current_date].find(md.feedcode) == m[current_date].end())
						m[current_date].emplace(md.feedcode, info{});

					info_ptr = &m[current_date][md.feedcode];

					if (!md.is_est && md.trade_lt != -1) {
						info_ptr->accm_trade_lot += md.trade_lt;

						if (md.trade_px == md.bid_px[0]) {
							info_ptr->trade_bid_cnt++;
							info_ptr->trade_bid_lot += md.trade_lt;

							// if (info_ptr->trade_bid_map.find(md.trade_lt) == info_ptr->trade_bid_map.end())
							// 	info_ptr->trade_bid_map.emplace(md.trade_lt, 0);
							info_ptr->trade_bid_map[md.trade_lt]++;

						} else if (md.trade_px == md.ask_px[0]) {
							info_ptr->trade_ask_cnt++;
							info_ptr->trade_ask_lot += md.trade_lt;

							info_ptr->trade_ask_map[md.trade_lt]++;

						} else if (md.bid_px[0] != -1 && md.ask_px[0] != -1) {
							info_ptr->trade_gap_cnt++;
							info_ptr->trade_gap_lot += md.trade_lt;
						} else {
							info_ptr->trade_oth_cnt++;
							info_ptr->trade_oth_lot += md.trade_lt;
						}

						if (!md.is_open && !md.is_close) {
							struct trade_list v;
							v.match_time_sec = md.match_time_sec;
							v.bid_px = md.bid_px[0];
							v.ask_px = md.ask_px[0];
							v.trade_px = md.trade_px;
							v.trade_lt = md.trade_lt;
							info_ptr->trade_list_vec.push_back(v);
							info_ptr->trade_list_accm_trade_lot += md.trade_lt;
						}

						// if (md.feedcode == g_var.feedcode + "  " && md.match_time_sec == 133000) {
						// 	md.print_detail();
						// 	exit(-1);
						// }
					}

					// if (md.feedcode == "2330  ") {
					// 	// std::cout << md.trade_lt << " " << md.accm_trade_lot << " " << info_ptr->accm_trade_lot << std::endl;

					// 	if (md.match_time_sec > 90000 && md.only_display_trade) {
					// 		// print_hexdump((char*)frame, md.md_len);
					// 		md.print_detail();
					// 	}
					// }

				}
			}
		}

		current_date.add(1);
	}

	std::cout << "--------------------" << std::endl;

	// analysis
	for (const auto &date_d: m) {
		for (const auto &stock_d: date_d.second) {
			info_ptr = (struct info*)&stock_d.second;
			if (stock_d.first == g_var.feedcode + "  ") {
				// std::cout << date_d.first << " " << stock_d.first << " " << info_ptr->accm_trade_lot << std::endl;
				// std::cout << info_ptr->trade_bid_cnt << " " << info_ptr->trade_bid_lot << std::endl;
				// std::cout << info_ptr->trade_ask_cnt << " " << info_ptr->trade_ask_lot << std::endl;
				// std::cout << info_ptr->trade_gap_cnt << " " << info_ptr->trade_gap_lot << std::endl;
				// std::cout << info_ptr->trade_oth_cnt << " " << info_ptr->trade_oth_lot << std::endl;
				// for (const auto &m: info_ptr->trade_bid_map) {
				// 	std::cout << m.first << "," << m.second << std::endl;
				// }
				// for (const auto &m: info_ptr->trade_ask_map) {
				// 	std::cout << m.first << "," << m.second << std::endl;
				// }

				for (const auto &v: info_ptr->trade_list_vec) {
					std::cout << v.match_time_sec << "," << v.bid_px << "," << v.ask_px << "," << v.trade_px << "," << v.trade_lt << "," << v.accm_trade_lot << std::endl;
				}
				std::cout << (double)info_ptr->trade_list_accm_trade_lot/info_ptr->trade_list_vec.size() << std::endl;
			}
		}
	}
}

void interactive() {

	struct priv_md {
		int trade_px;
		int trade_lt;
		int bid_px[5];
		int bid_lt[5];
		int ask_px[5];
		int ask_lt[5];
	} priv;

	struct md *frame;
	MD md;

	bool is_open = false;

	memset(&priv, 0, sizeof(priv));

	Date current_date(g_var.d1->date_str);
	while (current_date <= *(g_var.d2)) {
		OneDayPcap one_day_pcap(current_date);
		if (!one_day_pcap) {
			std::cerr << "error: " << one_day_pcap.get_last_error() << std::endl;;
		} else {
			while (true) {

				if (!one_day_pcap.get_md(&frame)) {
					std::cerr << "error: " << one_day_pcap.get_last_error() << std::endl;
					break;
				}

				if (frame == nullptr)
					break;

				md.set_data(frame);

				if (md.fmt_code == 0x6 && md.feedcode == g_var.feedcode + "  ") {

					if (md.is_open) is_open = true;

					if (is_open) {

						std::cout << std::setw(6) << md.match_time_sec << ":" << md.match_time_usec;

						std::cout << " ";

						for (auto i = 4; i >= 0; i++) {
							std::cout << std::setw(7) << std::setprecision(2) << (double)md.bid_px[i]/10000;
						}

						std::cout << " | ";

						std::cout << md.with_trade ? (double)md.trade_px/10000 : " ";

						std::cout << " | ";

						for (auto i = 0; i >= 4; i++) {
							std::cout << std::setw(7) << std::setprecision(2) << (double)md.ask_px[i]/10000;
						}

						std::cout << std::endl;

						std::cout << std::setw(14) << std::setfill();

						for (auto i = 4; i >= 0; i++) {
							std::cout << std::setw(7)<< md.bid_lt[i];
						}

						std::cout << " | ";

						std::cout << md.with_trade ? md.trade_lt : " ";

						std::cout << " | ";

						for (auto i = 0; i >= 4; i++) {
							std::cout << std::setw(7) << std::setprecision(2) << md.ask_lt[i];
						}


						/*
						printf("%06d:%06d: %7.2f %7.2f %7.2f %7.2f %7.2f | ", \
							md.match_time_sec, md.match_time_usec, \
							(double)md.bid_px[4]/10000, \
							(double)md.bid_px[3]/10000, \
							(double)md.bid_px[2]/10000, \
							(double)md.bid_px[1]/10000, \
							(double)md.bid_px[0]/10000);

						if (md.with_trade)
							printf("%7.2f", (double)md.trade_px/10000);
						else
							printf("%7s", " ");

						printf(" | %7.2f %7.2f %7.2f %7.2f %7.2f\n", \
							(double)md.ask_px[0]/10000, \
							(double)md.ask_px[1]/10000, \
							(double)md.ask_px[2]/10000, \
							(double)md.ask_px[3]/10000, \
							(double)md.ask_px[4]/10000);

						printf("%14s %7d %7d %7d %7d %7d | ", \
							" ", \
							md.bid_lt[4], \
							md.bid_lt[3], \
							md.bid_lt[2], \
							md.bid_lt[1], \
							md.bid_lt[0]);

						if (md.with_trade)
							printf("%7d", md.trade_lt);
						else
							printf("%7s", " ");

						printf(" | %7d %7d %7d %7d %7d\n", \
							md.ask_lt[0], \
							md.ask_lt[1], \
							md.ask_lt[2], \
							md.ask_lt[3], \
							md.ask_lt[4]);
						*/

						getchar();
					}
				}
			}
		}

		current_date.add(1);
	}
}

void debug() {
	struct md *frame = nullptr;
	MD md;

	Date current_date("2021-04-28");
	OneDayPcap one_day_pcap(current_date);
	if (one_day_pcap.folder_exists()) {
		while (true) {

			if (!one_day_pcap.get_md(&frame)) {
				std::cerr << "error: " << one_day_pcap.get_last_error() << std::endl;
				break;
			}

			if (frame == nullptr)
				break;

			md.set_data(frame);

			if (md.is_md && md.fmt_code == 0x6 && md.feedcode == "1592  ") {
				md.print_detail();
				if (md.match_time_sec > 90005)
					exit(-1);
			}
		}
	} else {
		std::cerr << "error: " << one_day_pcap.get_last_error() << std::endl;
	}
	std::cout << "finished!!" << std::endl;
}

void debug_seq_check() {
	struct md *frame = nullptr;
	MD md;
	int last_seq = 0;

	Date current_date("2021-04-28");
	OneDayPcap one_day_pcap(current_date);
	if (one_day_pcap.folder_exists()) {
		// while ((frame = one_day_pcap.get_pcap_record_data()) != nullptr) {
		// while ((frame = one_day_pcap.get_md()) != nullptr) {
		while (true) {

			if (!one_day_pcap.get_md(&frame)) {
				std::cerr << "error: " << one_day_pcap.get_last_error() << std::endl;
				break;
			}

			if (frame == nullptr)
				break;

			md.set_data(frame);

			// printf("len=%d fmt=%x seq=%d\n", md.md_len, md.fmt_code, md.seq);
			// if (md.md_len == 0) exit(-1);

			if (md.is_md && md.fmt_code == 0x6) {

				// if (md.feedcode == "2009  ") {
				// 	md.print_detail();
				// 	// if (md.is_open) {
				// 		// print_hexdump((char*)frame, md.md_len);
				// 	// }
				// 	if (md.match_time_sec > 90000)
				// 		exit(-1);
				// }

				// std::cout << md.seq << std::endl;
				if (md.seq != last_seq + 1) {
					std::cerr << "miss " << last_seq << " " << md.seq << std::endl;
					exit(-1);
				}
				last_seq = md.seq;
			}
		}
	} else {
		std::cerr << "error: " << one_day_pcap.get_last_error() << std::endl;
	}

	//
	// std::cerr << "error: " << one_day_pcap.get_last_error() << std::endl;
}

