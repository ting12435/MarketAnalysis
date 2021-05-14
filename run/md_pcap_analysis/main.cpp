#include <iomanip>
#include <map>
#include <iterator>
#include <cassert>
#include <cstring>
#include <getopt.h>
// #include <unistd.h>

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

	struct trades {
		int lt;
		int bid_lt;
		int ask_lt;
		int oth_lt;
	} trades;

	std::map<int, int> bid_pxlt_map, ask_pxlt_map;

	struct md *frame;
	MD md;

	// bool is_open = false;
	bool print_flag = false;

	memset(&trades, 0, sizeof(trades));

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

					// if (md.is_open) is_open = true;
					if (md.match_time_sec > 93300) print_flag = true;

					// if (is_open) {
					if (print_flag) {

						// print
						std::cout << md.get_match_time_str();

						std::cout << " |";

						std::cout << std::setfill(' ');

						// bid px
						for (auto i = 4; i >= 0; i--) {
							if (md.only_display_trade) {
								std::cout << std::setw(14) << " ";
							} else {
								if (md.bid_lt[i] != -1) {
									std::cout << std::setw(7) << std::fixed << std::setprecision(2) << (double)md.bid_px[i]/10000;
									std::cout << std::setw(7) << " ";
								} else {
									std::cout << std::setw(14) << " ";
								}
							}
							
						}

						std::cout << "| ";

						if (md.with_trade)
							std::cout << std::setw(7) << std::fixed << std::setprecision(2) << (double)md.trade_px/10000;
						else
							std::cout << std::setw(7) << " ";

						std::cout << " | ";

						// ask px
						for (auto i = 0; i <= 4; i++) {
							if (md.only_display_trade) {
								std::cout << std::setw(14) << " ";
							} else {
								if (md.ask_lt[i] != -1) {
									std::cout << std::setw(7) << std::fixed << std::setprecision(2) << (double)md.ask_px[i]/10000;
									std::cout << std::setw(7) << " ";
								} else {
									std::cout << std::setw(14) << " ";
								}
							}
						}

						std::cout << std::endl;

						std::cout << std::setw(17) << " ";

						// bid lt
						for (auto i = 4; i >= 0; i--) {

							if (md.only_display_trade) {
								std::cout << std::setw(14) << " ";
							} else {
								auto px = md.bid_px[i];
								auto lt = md.bid_lt[i];
								if (bid_pxlt_map.find(px) == bid_pxlt_map.end()) bid_pxlt_map[px] = 0;
								auto change_lt = lt - bid_pxlt_map[px];

								if (md.bid_lt[i] != -1) {
									std::cout << std::setw(7) << md.bid_lt[i];

									if (change_lt != 0 && change_lt != md.trade_lt*-1)
										std::cout << "(" << std::setw(5) << change_lt << ")";
									else
										std::cout << std::setw(7) << " ";
								} else {
									std::cout << std::setw(14) << " ";
								}

								bid_pxlt_map[px] = lt;
							}
						}

						std::cout << "| ";

						if (md.with_trade)
							std::cout << std::setw(7) << std::setw(7) << md.trade_lt;
						else
							std::cout << std::setw(7) << " ";

						std::cout << " | ";

						// ask lt
						for (auto i = 0; i <= 4; i++) {

							if (md.only_display_trade) {
								std::cout << std::setw(14) << " ";
							} else {
								auto px = md.ask_px[i];
								auto lt = md.ask_lt[i];
								if (ask_pxlt_map.find(px) == ask_pxlt_map.end()) ask_pxlt_map[px] = 0;
								auto change_lt = lt - ask_pxlt_map[px];
								
								if (md.ask_lt[i] != -1) {
									std::cout << std::setw(7) << md.ask_lt[i];

									if (change_lt != 0 && change_lt != md.trade_lt*-1)
										std::cout << "(" << std::setw(5) << change_lt << ")";
									else
										std::cout << std::setw(7) << " ";
								} else {
									std::cout << std::setw(14) << " ";
								}

								ask_pxlt_map[px] = lt;
							}
						}

						std::cout << "| ";

						// trades
						trades.lt += md.trade_lt;
						if (md.trade_px == md.bid_px[0])  trades.bid_lt += md.trade_lt;
						else if (md.trade_px == md.ask_px[0])  trades.ask_lt += md.trade_lt;
						else trades.oth_lt += md.trade_lt;

						std::cout << std::setw(7) << trades.lt << trades.bid_lt << trades.ask_lt;


						std::cout << std::endl;


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

