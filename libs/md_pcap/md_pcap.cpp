#include "md_pcap.h"
#include <cstring>

struct md* get_pcap_stream(Date) {
	return nullptr;
}

bool check_md_frame(struct md *md) {
	return false;
}


bool is_stock(struct md *md) { 
	return md->hdr.fmt_code == 0x06;
}

bool is_trade_uplimit(struct md *md) {
	return (md->body.fmt_6_17.limit_mark & 0xc0) == 0x80;
}

std::string get_feedcode(struct md *md) {
	char fc[FEEDCODE_SIZE];
	memcpy(fc, md->body.fmt_6_17.feedcode, FEEDCODE_SIZE);
	return std::string(fc);
}

struct md_px_lt* get_trade_pxlt(struct md *md) {
	return (struct md_px_lt*)(md->body.fmt_6_17.px_lt[0]);
}