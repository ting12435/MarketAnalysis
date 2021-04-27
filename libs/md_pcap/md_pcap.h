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

#define GET_FEEDOCDE(ptr) (std::string((char*)ptr, 6))
#define GET_PX(ptr) ( \
	(((char*)ptr) + 0) * 100000000 + \
	(((char*)ptr) + 1) * 1000000 + \
	(((char*)ptr) + 2) * 10000 + \
	(((char*)ptr) + 3) * 100 + \
	(((char*)ptr) + 4) * 1 \
)

extern std::string pcap_folder;
extern std::string pcap_market;

struct __attribute__((__packed__)) md_header {
	uint16_t 	msg_len;
	uint8_t 	market;
	uint8_t 	fmt_code;
	uint8_t 	fmt_ver;
	uint32_t 	seq;
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
	uint32_t 		accm_trade_lot;

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

class OneDayPcap {
public:
	OneDayPcap(Date);
	// ~OneDayPcap() = defalut;

	struct md* get_pcap_record_data();

	std::string get_error() { return this->error_ss.str(); }

	operator bool() const { return this->pcap_folder_exist; }

	Date date;
	std::string date_folder;

	char record_data[1518];

private:
	bool open_pcap_file(int idx);
	void close_pcap_file(int idx);

	int cur_pcap_idx;
	pcap_file *cur_pcap_file;
	std::string date_str;

	std::stringstream error_ss;

	bool pcap_folder_exist;
};

class MD {

public:
	bool set_data(struct md*);

	static void print_md(struct md*);

	bool is_md;

	// header
	int md_len;
	int market;
	int fmt_code;
	int fmt_ver;
	int seq;

	// body 6
	std::string feedcode;
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


//
bool check_md_frame(struct md*);

bool is_stock(struct md*);
bool with_trade_uplimit(struct md*);
std::string get_feedcode(struct md*);
struct md_px_lt* get_trade_pxlt(struct md*);
int get_px(struct md_px_lt*);
int get_lt(struct md_px_lt*);

#endif // MD_PCAP_H



