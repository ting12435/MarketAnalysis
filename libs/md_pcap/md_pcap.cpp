#include "md_pcap.h"
#include <cstring>
#include <arpa/inet.h>

std::string pcap_folder;
std::string pcap_market;

__attribute__((constructor)) void __md_pcap_init() {
	// std::cout << "a\n";
	// pcap_folder = "";
	// pcap_market = "";
	// std::cout << "a\n";
}

/********** OneDayPcap **********/
OneDayPcap::OneDayPcap(Date d) {
	this->date = d;
	this->cur_pcap_idx = -1;
	this->cur_pcap_file = nullptr;
	this->date_str = this->date.date_str.substr(0, 4) + this->date.date_str.substr(5, 2) + this->date.date_str.substr(8, 2);
	this->date_folder = pcap_folder + this->date_str;

	this->pcap_folder_exist = File::dir_exists(pcap_folder);
	if (!this->pcap_folder_exist)
		this->error_ss << "pcap folder not exist [" << pcap_folder << "]";

}

struct md* OneDayPcap::get_pcap_record_data() {

// std::cout << "get_pcap_record_data\n";

	// check file
	if (this->cur_pcap_idx == -1) {
		this->cur_pcap_idx = 29;  //
		this->open_pcap_file(++this->cur_pcap_idx);
	}

	if (this->cur_pcap_file->eof()) {
		this->close_pcap_file(this->cur_pcap_idx);
		this->open_pcap_file(++this->cur_pcap_idx);
	}

	if (this->cur_pcap_file->read(this->record_data, sizeof(this->record_data)) < 0) {
		this->error_ss << this->cur_pcap_file->get_error();
		return nullptr;
	}

// print_hexdump(this->record_data, sizeof(this->record_data));
// exit(-1);

	return (struct md*)((char*)&this->record_data + 42);
}

bool OneDayPcap::open_pcap_file(int idx) {
// std::cout << "open_pcap_file " << idx << std::endl;
	// TSE_20210423.pcap22
	std::stringstream fn_ss;
	fn_ss << this->date_folder << "/" << pcap_market << "_" << this->date_str << ".pcap" << (idx == 0 ? "" : std::to_string(idx));
	this->cur_pcap_file = new pcap_file(fn_ss.str());
	if (!*(this->cur_pcap_file)) {
		this->error_ss << this->cur_pcap_file->get_error();
		return false;
	}
	return true;
}

void OneDayPcap::close_pcap_file(int idx) {
	delete this->cur_pcap_file;
}

/********** MD **********/
bool MD::set_data(struct md *md_ptr) {
	uint8_t c;
	struct md_px_lt *px_lt_ptr;

	this->clear();

	this->is_md = md_ptr->esc_code == 27;

	this->md_len = htons(md_ptr->hdr.msg_len);
	this->market = md_ptr->hdr.market;
	this->fmt_code = md_ptr->hdr.fmt_code;
	this->fmt_ver = md_ptr->hdr.fmt_ver;
	this->seq = htonl(md_ptr->hdr.seq);

	switch (md_ptr->hdr.fmt_code) {
		case 0x01:
			this->feedcode = GET_FEEDOCDE(md_ptr->body.fmt_1.feedcode);
			break;
		case 0x06:
			this->feedcode = GET_FEEDOCDE(md_ptr->body.fmt_6_17.feedcode);
			c = md_ptr->body.fmt_6_17.display_mark;
			this->with_trade 			= (c >> 7) & 0x1;
			this->b_cnt 				= (c >> 4) & 0x7;
			this->s_cnt 				= (c >> 1) & 0x7;
			this->only_display_trade 	= (c >> 0) & 0x1;
			c = md_ptr->body.fmt_6_17.limit_mark;
			this->trade_limit 			= (c >> 6) & 0x3;
			this->b_limit 				= (c >> 4) & 0x3;
			this->s_limit 				= (c >> 2) & 0x3;
			this->fast_price 			= (c >> 0) & 0x3;
			c = md_ptr->body.fmt_6_17.status_mark;
			this->is_est 				= (c >> 7) & 0x1;  // false: 一般揭示, true: 試算揭示
			this->is_est_delay_open 	= (c >> 6) & 0x1;
			this->is_est_delay_close 	= (c >> 5) & 0x1;
			this->match_mode 			= (c >> 4) & 0x1;  // false: 集合競價, true: 逐筆撮合
			this->is_open 				= (c >> 3) & 0x1;
			this->is_close 				= (c >> 2) & 0x1;

			this->accm_trade_lot = htonl(md_ptr->body.fmt_6_17.accm_trade_lot);

			px_lt_ptr = (struct md_px_lt*)((char*)&md_ptr->body.fmt_6_17.accm_trade_lot + 1);

			if (this->with_trade) {
				this->trade_px = GET_PX(px_lt_ptr->px);
				this->trade_lt = -1;
			}
			
			for (auto i = 0; i < 5; i++) {
				this->bid_px[i] = -1;
				this->bid_lt[i] = -1;
				this->ask_px[i] = -1;
				this->ask_lt[i] = -1;
			}

			break;
	}

	
	
	

	this->vaild = false;
}

void MD::print_md(struct md *md_ptr) {
	std::stringstream ss;
	char buf[100];
	std::string field;
	uint8_t c;

	if (md_ptr->esc_code != 27) {
		ss << "not md" << std::endl;
		goto print;
	}

	field = "esc_code";
	snprintf(buf, sizeof(buf), "%s: 0x%02x (%u)", field.c_str(), md_ptr->esc_code, md_ptr->esc_code);
	ss << buf << std::endl;

	// header
	field = "msg_len";
	snprintf(buf, sizeof(buf), "%s: %x", field.c_str(), htons(md_ptr->hdr.msg_len));
	ss << buf << std::endl;

	field = "market";
	snprintf(buf, sizeof(buf), "%s: %02x", field.c_str(), md_ptr->hdr.market);
	ss << buf << std::endl;

	field = "fmt_code";
	snprintf(buf, sizeof(buf), "%s: %02x", field.c_str(), md_ptr->hdr.fmt_code);
	ss << buf << std::endl;

	field = "fmt_ver";
	snprintf(buf, sizeof(buf), "%s: %02x", field.c_str(), md_ptr->hdr.fmt_ver);
	ss << buf << std::endl;

	field = "seq";
	snprintf(buf, sizeof(buf), "%s: %x", field.c_str(), htonl(md_ptr->hdr.seq));
	ss << buf << std::endl;

	switch (md_ptr->hdr.fmt_code) {
		case 0x01:
			field = "feedcode";
			snprintf(buf, sizeof(buf), "%s: %s", field.c_str(), GET_FEEDOCDE(md_ptr->body.fmt_1.feedcode).c_str());
			ss << buf << std::endl;
			break;
		case 0x06:
			field = "feedcode";
			snprintf(buf, sizeof(buf), "%s: %s", field.c_str(), GET_FEEDOCDE(md_ptr->body.fmt_6_17.feedcode).c_str());
			ss << buf << std::endl;

			field = "display_mark";
			c = md_ptr->body.fmt_6_17.display_mark;
			snprintf(buf, sizeof(buf), "%s: %02x (%1x %3x %3x %1x)", field.c_str(), c,
				(c >> 7) & 0x1,
				(c >> 4) & 0x7,
				(c >> 1) & 0x7,
				(c >> 0) & 0x1);
			ss << buf << std::endl;

			field = "limit_mark";
			c = md_ptr->body.fmt_6_17.limit_mark;
			snprintf(buf, sizeof(buf), "%s: %02x (%02x %02x %02x %02x)", field.c_str(), c,
				(c >> 6) & 0x3,
				(c >> 4) & 0x3,
				(c >> 2) & 0x3,
				(c >> 0) & 0x3);
			ss << buf << std::endl;

			field = "status_mark";
			c = md_ptr->body.fmt_6_17.status_mark;
			snprintf(buf, sizeof(buf), "%s: %02x (%1x %1x %1x %1x %1x %1x %02x)", field.c_str(), c,
				(c >> 7) & 0x1,
				(c >> 6) & 0x1,
				(c >> 5) & 0x1,
				(c >> 4) & 0x1,
				(c >> 3) & 0x1,
				(c >> 2) & 0x1,
				(c >> 0) & 0x3);
			ss << buf << std::endl;

			field = "accm_trade_lot";
			snprintf(buf, sizeof(buf), "%s: %x", field.c_str(), htonl(md_ptr->body.fmt_6_17.accm_trade_lot));
			ss << buf << std::endl;

			// px_lt
			// md_ptr->body.fmt_6_17.px_lt = (struct md_px_lt**)((char*)&md_ptr->body.fmt_6_17.accm_trade_lot + 1);


			break;
	}

	print:
	std::cout << ss.str();
}

void MD::clear() {
	this->is_md = false;

	this->md_len = -1;
	this->market = -1;
	this->fmt_code = -1;
	this->fmt_ver = -1;
	this->seq = -1;

	this->feedcode = "";
	this->with_trade = false;
	this->b_cnt = -1;
	this->s_cnt = -1;
	this->only_display_trade = false;
	this->trade_limit = -1;
	this->b_limit = -1;
	this->s_limit = -1;
	this->fast_price = -1;
	this->is_est = false;  // false: 一般揭示, true: 試算揭示
	this->is_est_delay_open = false;
	this->is_est_delay_close = false;
	this->match_mode = false;  // false: 集合競價, true: 逐筆撮合
	this->is_open = false;
	this->is_close = false;

	this->accm_trade_lot = -1;
	this->trade_px = -1;
	this->trade_lt = -1;
	for (auto i = 0; i < 5; i++) {
		this->bid_px[i] = -1;
		this->bid_lt[i] = -1;
		this->ask_px[i] = -1;
		this->ask_lt[i] = -1;
	}
	

	this->vaild = false;
}

bool check_md_frame(struct md *md) {
	if (md->esc_code != 27) return false;
	return true;
}


bool is_stock(struct md *md) { 
	return md->hdr.fmt_code == 0x06;
}

bool with_trade_uplimit(struct md *md) {
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

int get_px(struct md_px_lt *px_lt) {
	return bcd_to_int(px_lt->px, MD_PX_SIZE) / 10000.0;
}

int get_lt(struct md_px_lt *px_lt) {
	return bcd_to_int(px_lt->lt, MD_LT_SIZE);
}