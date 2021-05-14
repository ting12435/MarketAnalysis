#ifndef MD_PCAP_H
#define MD_PCAP_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <cmath>

#include "util/util.h"
#include "util/market_util.h"

#define FEEDCODE_SIZE 6
#define MD_PX_SIZE 5
#define MD_LT_SIZE 4

#define bcd_to_int(buf, len) ({ \
	int i, dec_pow = 1 + 2 * (len - 1); \
	int sum = 0; \
	for (i = 0; i < len; i++) { \
		sum += (buf[i] >> 4 & 0x0f) * pow(10,dec_pow--); \
		sum += (buf[i] & 0x0f) * pow(10,dec_pow--); \
	} \
	sum; \
})

#define bcd_to_long(buf, len) ({ \
	int i, dec_pow = 1 + 2 * (len - 1); \
	long sum = 0; \
	for (i = 0; i < len; i++) { \
		sum += (buf[i] >> 4 & 0x0f) * pow(10,dec_pow--); \
		sum += (buf[i] & 0x0f) * pow(10,dec_pow--); \
	} \
	sum; \
})

#define GET_FEEDOCDE(ptr) (std::string((char*)ptr, 6))
#define GET_PX(ptr) bcd_to_long(ptr, MD_PX_SIZE)
#define GET_LT(ptr) bcd_to_long(ptr, MD_LT_SIZE)
/*#define GET_PX(ptr) ( \
	(*(((uint8_t*)ptr) + 0) >> 4 ) * (long)1000000000 + \
	(*(((uint8_t*)ptr) + 0) & 0xf) * (long)100000000 + \
	(*(((uint8_t*)ptr) + 1) >> 4 ) * (long)10000000 + \
	(*(((uint8_t*)ptr) + 1) & 0xf) * (long)1000000 + \
	(*(((uint8_t*)ptr) + 2) >> 4 ) * (long)100000 + \
	(*(((uint8_t*)ptr) + 2) & 0xf) * (long)10000 + \
	(*(((uint8_t*)ptr) + 3) >> 4 ) * (long)1000 + \
	(*(((uint8_t*)ptr) + 3) & 0xf) * (long)100 + \
	(*(((uint8_t*)ptr) + 4) >> 4 ) * (long)10 + \
	(*(((uint8_t*)ptr) + 4) & 0xf) * (long)1 \
)
#define GET_LT(ptr) ( \
	(*(((uint8_t*)ptr) + 0) >> 4 ) * 10000000 + \
	(*(((uint8_t*)ptr) + 0) & 0xf) * 1000000 + \
	(*(((uint8_t*)ptr) + 1) >> 4 ) * 100000 + \
	(*(((uint8_t*)ptr) + 1) & 0xf) * 10000 + \
	(*(((uint8_t*)ptr) + 2) >> 4 ) * 1000 + \
	(*(((uint8_t*)ptr) + 2) & 0xf) * 100 + \
	(*(((uint8_t*)ptr) + 3) >> 4 ) * 10 + \
	(*(((uint8_t*)ptr) + 4) & 0xf) * 1 \
)*/

extern std::string pcap_folder;
extern std::string pcap_market;

struct __attribute__((__packed__)) md_header {
	uint8_t 	msg_len[2];
	uint8_t 	market;
	uint8_t 	fmt_code;
	uint8_t 	fmt_ver;
	uint8_t 	seq[4];
};

struct __attribute__((__packed__)) md_px_lt {
	uint8_t px[MD_PX_SIZE];
	uint8_t lt[MD_LT_SIZE];
};

struct __attribute__((__packed__)) md_body_fmt_1 {
	uint8_t 		feedcode[FEEDCODE_SIZE];
	uint32_t : 32;
	uint32_t : 32;
	uint32_t : 32;
	uint32_t : 32;
	uint32_t : 32;
	uint16_t 		stocks_count_mark[2];
};

struct __attribute__((__packed__)) md_body_fmt_6_17 {
	
	uint8_t 		feedcode[FEEDCODE_SIZE];
	uint8_t 		match_time[6];
	uint8_t 		display_mark;
	uint8_t 		limit_mark;
	uint8_t 		status_mark;
	uint8_t 		accm_trade_lot[4];

	// struct md_px_lt **px_lt;;
	// uint8_t 		*check_code;
};

struct __attribute__((__packed__)) md {
	uint8_t esc_code;

	struct md_header hdr;

	union {
		struct md_body_fmt_1 	fmt_1;
		struct md_body_fmt_6_17 fmt_6_17;
	} body;

	uint16_t *terminal_code;
};


class OneDayPcap {
public:
	OneDayPcap(Date);
	// ~OneDayPcap() = defalut;

	// struct md* get_md();
	bool get_md(struct md**);  // false: error occurs, ptr==nullptr: read finished
	// struct md* get_pcap_record_data();

	bool folder_exists();

	// std::string get_error() { return this->error_ss.str(); }
	std::string get_last_error() { return this->last_error; }

	operator bool() { return this->folder_exists(); }

	Date date;
	std::string date_folder;

	char record_data[1518];

private:
	bool open_pcap_file(int idx);
	void close_pcap_file(int idx);
	int get_pcap_record_data();

	int cur_pcap_idx;
	pcap_file *cur_pcap_file;
	std::string date_str;

	// std::stringstream error_ss;
	std::string last_error;

	// bool pcap_folder_exist;

	char *record_data_st_ptr;
	char *record_data_ed_ptr;
};

class MD {
public:
	void set_data(struct md*);
	void print_detail();

	std::string get_match_time_str();

	// static void print_md(struct md*);

	bool is_md;

	// header
	int md_len;
	int market;
	int fmt_code;
	int fmt_ver;
	int seq;

	// body 6
	std::string feedcode;
	int match_time_sec;
	int match_time_usec;
	// display mark
	bool with_trade;
	int b_cnt;
	int s_cnt;
	bool only_display_trade;  // false: 揭示成交價量與最佳五檔買賣價量, true: 僅揭示成交價量、不揭示最佳五檔買賣價量
	// limit mark
	int trade_limit;
	int b_limit;
	int s_limit;
	int fast_price;
	// status mark
	bool is_est;  // false: 一般揭示, true: 試算揭示
	bool is_est_delay_open;
	bool is_est_delay_close;
	bool match_mode;  // false: 集合競價, true: 逐筆撮合
	bool is_open;
	bool is_close;

	int accm_trade_lot;
	int trade_px;
	int trade_lt;
	int bid_px[5];
	int bid_lt[5];
	int ask_px[5];
	int ask_lt[5];

	bool vaild;

private:
	void clear();
};


#endif // MD_PCAP_H



