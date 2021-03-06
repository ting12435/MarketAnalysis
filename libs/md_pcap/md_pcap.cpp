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
	this->cur_pcap_idx = 0;
	this->cur_pcap_file = nullptr;
	this->date_str = this->date.date_str.substr(0, 4) + this->date.date_str.substr(5, 2) + this->date.date_str.substr(8, 2);
	// if (this->date_str == "20210506")
	// 	this->date_folder = pcap_folder + this->date_str + "_1330";
	// else
		this->date_folder = pcap_folder + this->date_str;

	this->record_data_st_ptr = nullptr;
	this->record_data_ed_ptr = nullptr;

	// 
	// this->cur_pcap_idx = 85;
}

bool OneDayPcap::folder_exists() {
	if (!File::dir_exists(this->date_folder)) {
		this->last_error = "folder not exists [" + this->date_folder + "]";
		return false;
	}
	return true;
	// this->pcap_folder_exist = File::dir_exists(this->date_folder);
	// // std::cout << date_folder << " " << this->pcap_folder_exist << std::endl;
	// if (!this->pcap_folder_exist) {
	// 	this->error_ss << "pcap folder not exist [" << this->date_folder << "]";
	// }
}

bool OneDayPcap::get_md(struct md **md_ptr) {
// printf("md_ptr=%p\n", md_ptr);
	*md_ptr = nullptr;

	if (this->record_data_st_ptr == nullptr || this->record_data_ed_ptr == nullptr) {

		int record_data_len;
		struct md *_md_ptr;
		while (true) {
			record_data_len = this->get_pcap_record_data();
			if (record_data_len < 0) {
				if (record_data_len == -1) {
					this->last_error = "read pcap error [" + this->cur_pcap_file->filename + "]";
					return false;
				} else if (record_data_len == -2) {
					return true;
				}
			} else if (record_data_len > 42) {
				_md_ptr = (struct md*)(this->record_data + 42);
				if (_md_ptr->esc_code == 27)
					break;
			}
		}

		this->record_data_st_ptr = this->record_data + 42;
		this->record_data_ed_ptr = this->record_data + record_data_len - 4;

		// printf("%d record_data_len=%d fmt=%x seq=%d\n", this->cur_pcap_idx, record_data_len, _md_ptr->hdr.fmt_code, bcd_to_int(_md_ptr->hdr.seq, 4));
	}

// if (record_data_len == 0) {
// printf("record_data_len=%d\n", record_data_len);
// print_hexdump(this->record_data, record_data_len);
// exit(-1);
// }
// printf("---------------------------------\n");

// printf("record_data_st_ptr=%p\n", this->record_data_st_ptr);
// printf("record_data_ed_ptr=%p\n", this->record_data_ed_ptr);

	*md_ptr = (struct md*)this->record_data_st_ptr;
	int msg_len = bcd_to_int((*md_ptr)->hdr.msg_len, 2);
	if (msg_len == 0) {
		// error
		// print_hexdump(this->record_data, this->record_data_ed_ptr - this->record_data_st_ptr + 1);
		this->last_error = "msg_len=0 [" + this->cur_pcap_file->filename + "]";
		return false;
	}
	this->record_data_st_ptr += msg_len;

	if (this->record_data_st_ptr >= this->record_data_ed_ptr ||
		*(this->record_data_st_ptr) != 27) {
		this->record_data_st_ptr = nullptr;
		this->record_data_ed_ptr = nullptr;
	}

// printf("msg_len=%d\n", msg_len);
// printf("record_data_st_ptr=%p\n", this->record_data_st_ptr);
// printf("record_data_ed_ptr=%p\n", this->record_data_ed_ptr);
// printf("p=%p\n", p);
// if (record_data_len == 0)
// exit(-1);
// printf("md_ptr=%p\n", md_ptr);
	return true;
}

int OneDayPcap::get_pcap_record_data() {
	/*
		return:
			>0: read success
			-1: read error
			-2: open error (read finished)
	*/

	int record_data_len = -1;

	// check file
	// if (this->cur_pcap_idx == -1) {
	if (this->cur_pcap_file == nullptr) {
		if (!this->open_pcap_file(this->cur_pcap_idx)) {
			this->last_error = this->cur_pcap_file->get_last_error();
			return -2;
		}
	}

	while ((record_data_len = this->cur_pcap_file->read(this->record_data, sizeof(this->record_data))) <= 0) {

		// this->error_ss << this->cur_pcap_file->get_error();

		this->close_pcap_file(this->cur_pcap_idx);

		if (!this->open_pcap_file(++this->cur_pcap_idx)) {
			// this->last_error << this->cur_pcap_file->get_last_error();
			return -2;
		}
	}

	return record_data_len;
}

//
// struct md* OneDayPcap::get_pcap_record_data() {

// 	// check file
// 	if (this->cur_pcap_idx == -1) {
// 		if (!this->open_pcap_file(++this->cur_pcap_idx)) {
// 			this->error_ss << this->cur_pcap_file->get_error();
// 			return nullptr;
// 		}
// 	}

// 	while (this->cur_pcap_file->read(this->record_data, sizeof(this->record_data)) < 0) {

// 		this->error_ss << this->cur_pcap_file->get_error();

// 		this->close_pcap_file(this->cur_pcap_idx);

// 		if (!this->open_pcap_file(++this->cur_pcap_idx)) {
// 			this->error_ss << this->cur_pcap_file->get_error();
// 			return nullptr;
// 		}
// 	}

// 	return (struct md*)((char*)&this->record_data + 42);
// }

bool OneDayPcap::open_pcap_file(int idx) {
// std::cout << this->date << " open_pcap_file " << idx << std::endl;
	// TSE_20210423.pcap22
	std::stringstream fn_ss;
	fn_ss << this->date_folder << "/" << pcap_market << "_" << this->date_str << ".pcap" << (idx == 0 ? "" : std::to_string(idx));

	std::string fn_str = fn_ss.str();

	if (!File::file_exists(fn_str)) {
		this->last_error = "file not exists [" + fn_str + "]";
		return false;
	}

std::cout << fn_str << std::endl;
	this->cur_pcap_file = new pcap_file();
	if (this->cur_pcap_file->open(fn_str) < 0) {
		this->last_error = this->cur_pcap_file->get_last_error();;
		return false;
	}
// 	if (!*(this->cur_pcap_file)) {
// 		this->error_ss << this->cur_pcap_file->get_error();
// // std::cout << this->error_ss.str() << std::endl;
// 		return false;
// 	}

	return true;
}

void OneDayPcap::close_pcap_file(int idx) {
	delete this->cur_pcap_file;
}

/********** MD **********/
void MD::set_data(struct md *md_ptr) {
	uint8_t c;
	struct md_px_lt *px_lt_ptr;
	uint8_t *p;

	this->clear();

	this->is_md = md_ptr->esc_code == 27;

	this->md_len = bcd_to_int(md_ptr->hdr.msg_len, 2);
	this->market = md_ptr->hdr.market;
	this->fmt_code = md_ptr->hdr.fmt_code;
	this->fmt_ver = md_ptr->hdr.fmt_ver;
	this->seq = bcd_to_int(md_ptr->hdr.seq, 4);

	switch (md_ptr->hdr.fmt_code) {
		case 0x01:
			this->feedcode = GET_FEEDOCDE(md_ptr->body.fmt_1.feedcode);
			break;
		case 0x06:
			this->feedcode = GET_FEEDOCDE(md_ptr->body.fmt_6_17.feedcode);
			p = md_ptr->body.fmt_6_17.match_time;
			this->match_time_sec = bcd_to_int(p, 3);
			p = &(md_ptr->body.fmt_6_17.match_time[3]);
			this->match_time_usec = bcd_to_int(p, 3);
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

			this->accm_trade_lot = bcd_to_int(md_ptr->body.fmt_6_17.accm_trade_lot, 4);

			px_lt_ptr = (struct md_px_lt*)((char*)&md_ptr->body.fmt_6_17.accm_trade_lot + 4);

			if (this->with_trade) {
				this->trade_px = GET_PX(px_lt_ptr->px);
				this->trade_lt = GET_LT(px_lt_ptr->lt);
				px_lt_ptr++;
			} 
			// else {
			// 	this->trade_px = 0;
			// 	this->trade_lt = 0;
			// }
			
			for (auto i = 0; i < this->b_cnt; i++) {
				this->bid_px[i] = GET_PX(px_lt_ptr->px);
				this->bid_lt[i] = GET_LT(px_lt_ptr->lt);
				px_lt_ptr++;
			}

			for (auto i = 0; i < this->s_cnt; i++) {
				this->ask_px[i] = GET_PX(px_lt_ptr->px);
				this->ask_lt[i] = GET_LT(px_lt_ptr->lt);
				px_lt_ptr++;
			}

			break;
	}

	this->vaild = true;
}

void MD::print_detail() {
	std::stringstream ss;
	char buf[100];

	ss << "is_md: " << this->is_md << std::endl;

	if (this->is_md) {
		ss << "md_len: " << this->md_len << std::endl;
		ss << "market: " << this->market << std::endl;
		ss << "fmt_code: " << this->fmt_code << std::endl;
		ss << "fmt_ver: " << this->fmt_ver << std::endl;
		ss << "seq: " << this->seq << std::endl;
		switch (this->fmt_code) {
			case 1:
				ss << "feedcode: " << this->feedcode << std::endl;
				break;
			case 6:
				ss << "feedcode: " << this->feedcode << std::endl;
				snprintf(buf, sizeof(buf), "%06d.%06d", this->match_time_sec, this->match_time_usec);
				ss << "match_time: " << buf << std::endl;
				ss << "with_trade: " << this->with_trade << std::endl;
				ss << "b_cnt: " << this->b_cnt << std::endl;
				ss << "s_cnt: " << this->s_cnt << std::endl;
				ss << "only_display_trade: " << this->only_display_trade << std::endl;
				ss << "trade_limit: " << this->trade_limit << std::endl;
				ss << "b_limit: " << this->b_limit << std::endl;
				ss << "s_limit: " << this->s_limit << std::endl;
				ss << "fast_price: " << this->fast_price << std::endl;
				ss << "is_est: " << this->is_est << std::endl;
				ss << "is_est_delay_open: " << this->is_est_delay_open << std::endl;
				ss << "is_est_delay_close: " << this->is_est_delay_close << std::endl;
				ss << "match_mode: " << this->match_mode << std::endl;
				ss << "is_open: " << this->is_open << std::endl;
				ss << "is_close: " << this->is_close << std::endl;
				ss << "accm_trade_lot: " << this->accm_trade_lot << std::endl;
				ss << "trade_px: " << this->trade_px << std::endl;
				ss << "trade_lt: " << this->trade_lt << std::endl;
				for (auto i = 0; i < 5; i++) {
					ss << "bid_px_" << i + 1 << ": " << this->bid_px[i] << std::endl;
					ss << "bid_lt_" << i + 1 << ": " << this->bid_lt[i] << std::endl;
				}
				for (auto i = 0; i < 5; i++) {
					ss << "ask_px_" << i + 1 << ": " << this->ask_px[i] << std::endl;
					ss << "ask_lt_" << i + 1 << ": " << this->ask_lt[i] << std::endl;
				}
				
				break;
		}
	}

	ss << "vaild: " << this->vaild << std::endl;

	ss << "-------------------------------------------" << std::endl;

	std::cout << ss.str();
}

std::string MD::get_match_time_str() {
	char buf[16];  // 00:00:00.123456
	snprintf(buf, sizeof(buf), "%02d:%02d:%02d.%06d", 
		this->match_time_sec/10000,
		(this->match_time_sec/100) % 100,
		this->match_time_sec % 100,
		this->match_time_usec);
	return std::string(buf);
}

// void MD::print_md(struct md *md_ptr) {
	// std::stringstream ss;
	// char buf[100];
	// std::string field;
	// uint8_t c;

	// if (md_ptr->esc_code != 27) {
	// 	ss << "not md" << std::endl;
	// 	goto print;
	// }

	// field = "esc_code";
	// snprintf(buf, sizeof(buf), "%s: 0x%02x (%u)", field.c_str(), md_ptr->esc_code, md_ptr->esc_code);
	// ss << buf << std::endl;

	// // header
	// field = "msg_len";
	// snprintf(buf, sizeof(buf), "%s: %x", field.c_str(), htons(md_ptr->hdr.msg_len));
	// ss << buf << std::endl;

	// field = "market";
	// snprintf(buf, sizeof(buf), "%s: %02x", field.c_str(), md_ptr->hdr.market);
	// ss << buf << std::endl;

	// field = "fmt_code";
	// snprintf(buf, sizeof(buf), "%s: %02x", field.c_str(), md_ptr->hdr.fmt_code);
	// ss << buf << std::endl;

	// field = "fmt_ver";
	// snprintf(buf, sizeof(buf), "%s: %02x", field.c_str(), md_ptr->hdr.fmt_ver);
	// ss << buf << std::endl;

	// field = "seq";
	// snprintf(buf, sizeof(buf), "%s: %x", field.c_str(), htonl(md_ptr->hdr.seq));
	// ss << buf << std::endl;

	// switch (md_ptr->hdr.fmt_code) {
	// 	case 0x01:
	// 		field = "feedcode";
	// 		snprintf(buf, sizeof(buf), "%s: %s", field.c_str(), GET_FEEDOCDE(md_ptr->body.fmt_1.feedcode).c_str());
	// 		ss << buf << std::endl;
	// 		break;
	// 	case 0x06:
	// 		field = "feedcode";
	// 		snprintf(buf, sizeof(buf), "%s: %s", field.c_str(), GET_FEEDOCDE(md_ptr->body.fmt_6_17.feedcode).c_str());
	// 		ss << buf << std::endl;

	// 		field = "display_mark";
	// 		c = md_ptr->body.fmt_6_17.display_mark;
	// 		snprintf(buf, sizeof(buf), "%s: %02x (%1x %3x %3x %1x)", field.c_str(), c,
	// 			(c >> 7) & 0x1,
	// 			(c >> 4) & 0x7,
	// 			(c >> 1) & 0x7,
	// 			(c >> 0) & 0x1);
	// 		ss << buf << std::endl;

	// 		field = "limit_mark";
	// 		c = md_ptr->body.fmt_6_17.limit_mark;
	// 		snprintf(buf, sizeof(buf), "%s: %02x (%02x %02x %02x %02x)", field.c_str(), c,
	// 			(c >> 6) & 0x3,
	// 			(c >> 4) & 0x3,
	// 			(c >> 2) & 0x3,
	// 			(c >> 0) & 0x3);
	// 		ss << buf << std::endl;

	// 		field = "status_mark";
	// 		c = md_ptr->body.fmt_6_17.status_mark;
	// 		snprintf(buf, sizeof(buf), "%s: %02x (%1x %1x %1x %1x %1x %1x %02x)", field.c_str(), c,
	// 			(c >> 7) & 0x1,
	// 			(c >> 6) & 0x1,
	// 			(c >> 5) & 0x1,
	// 			(c >> 4) & 0x1,
	// 			(c >> 3) & 0x1,
	// 			(c >> 2) & 0x1,
	// 			(c >> 0) & 0x3);
	// 		ss << buf << std::endl;

	// 		field = "accm_trade_lot";
	// 		snprintf(buf, sizeof(buf), "%s: %x", field.c_str(), htonl(md_ptr->body.fmt_6_17.accm_trade_lot));
	// 		ss << buf << std::endl;

	// 		// px_lt
	// 		// md_ptr->body.fmt_6_17.px_lt = (struct md_px_lt**)((char*)&md_ptr->body.fmt_6_17.accm_trade_lot + 1);
	// 		break;
	// }

	// print:
	// std::cout << ss.str();
// }

void MD::clear() {
	this->is_md = false;

	this->md_len = -1;
	this->market = -1;
	this->fmt_code = -1;
	this->fmt_ver = -1;
	this->seq = -1;

	this->feedcode = "";
	this->match_time_sec = -1;
	this->match_time_usec = -1;
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
