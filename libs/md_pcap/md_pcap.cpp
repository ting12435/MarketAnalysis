#include "md_pcap.h"
#include <cstring>

std::string pcap_folder;
std::string pcap_market;

__attribute__((constructor)) void __md_pcap_init() {
	// std::cout << "a\n";
	// pcap_folder = "";
	// pcap_market = "";
	// std::cout << "a\n";
}

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
	// check file
	if (this->cur_pcap_idx == -1)
		this->open_pcap_file(++this->cur_pcap_idx);

	if (this->cur_pcap_file->eof())
		this->open_pcap_file(++this->cur_pcap_idx);

	if (this->cur_pcap_file->read(this->record_data, sizeof(this->record_data)) < 0) {
		this->error_ss << this->cur_pcap_file->get_error();
		return nullptr;
	}
	return (struct md*)&this->record_data;
}

bool OneDayPcap::open_pcap_file(int idx) {
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

// struct md* get_pcap_stream(Date d) {
// 	return nullptr;
// }

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

int get_px(struct md_px_lt *px_lt) {
	return bcd_to_int(px_lt->px, MD_PX_SIZE) / 10000.0;
}

int get_lt(struct md_px_lt *px_lt) {
	return bcd_to_int(px_lt->lt, MD_LT_SIZE);
}