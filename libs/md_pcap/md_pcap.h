#ifndef MD_PCAP_H
#define MD_PCAP_H

#include <iostream>
#include <string>
#include <map>
#include <vector>

#include "util/util.h"
#include "util/market_util.h"

#define FEEDCODE_SIZE 6
#define MD_PX_SIZE 5
#define MD_LT_SIZE 4

#define IS_STOCK(md) (md->hdr.fmt_code == 0x06)
#define IS_TRADE_UPLIMIT(md) (md->body.fmt_6_17.limit_mark & 0xc0 == 0x80)
#define GET_FEEDCODE(md) ((char*)md->body.fmt_6_17.feedcode)
#define GET_TRADE_PXLT(md) ((char*)md->body.fmt_6_17.px_lt[0])

struct md_header {
	uint16_t 	msg_len;
	uint8_t 	market;
	uint8_t 	fmt_code;
	uint8_t 	fmt_ver;
	uint32_t 	seq;
};

struct md_px_lt {
	uint8_t px[MD_PX_SIZE];
	uint8_t lt[MD_LT_SIZE];
};

struct md_body_fmt_1 {
	uint8_t 		feedcode[FEEDCODE_SIZE];
	uint32_t : 160;
	uint16_t 		stocks_count_mark[2];
};

struct md_body_fmt_6_17 {
	
	uint8_t 		feedcode[FEEDCODE_SIZE];
	uint8_t 		match_time[6];
	uint8_t 		show_mark;
	uint8_t 		limit_mark;
	uint8_t 		status_mark;
	uint32_t 		accm_trade_lot;

	struct md_px_lt **px_lt;;
	uint8_t 		*check_code;
};

struct md {
	uint8_t esc_code;

	struct md_header hdr;

	union {
		struct md_body_fmt_1 	fmt_1;
		struct md_body_fmt_6_17 fmt_6_17;
	} body;

	uint16_t *terminal_code;
};

struct md* get_pcap_stream(Date);

bool check_md_frame(struct md*);

// int parse_md_pcap_frame(char*, int);

#endif // MD_PCAP_H



