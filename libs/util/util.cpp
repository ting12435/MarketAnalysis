#include "util.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdio> 
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>

std::vector<std::string> split(const std::string& s, std::string delim, int split_cnt) {
	std::vector<std::string> sv;
    int i;
    std::string temp;
    std::istringstream iss(s);

    i = 0;
    auto start = 0U;
    auto end = s.find(delim);
    while (end != std::string::npos) {

       // if (split_cnt <= 0) break;

        sv.emplace_back(s.substr(start, end - start));
        start = end + delim.length();
        end = s.find(delim, start);

        i++;
        if (split_cnt > 0 && i >= split_cnt) break;
    }
    end = s.size();
    sv.emplace_back(s.substr(start, end - start));

    return sv;
}

std::string only_number_and_str(const std::string& s) {
	std::string result_s;
	for (const auto &c: s) {
		if ((int(c) >= 48 && int(c) <= 57) ||
			(int(c) >= 65 && int(c) <= 90) ||
			(int(c) >= 97 && int(c) <= 122))
			result_s += c;
	}
	return result_s;
}

void output_csv(std::ostream& os, std::initializer_list<std::string> args) {
	output_delimiter_str(os, ",", args);
}

void output_delimiter_str(std::ostream& os, std::string delimiter, std::initializer_list<std::string> args) {
	bool f = false;
	for (const auto &arg: args) {
		if (f) os << delimiter;
		os << arg;
		f = true;
	}
	os << std::endl;
}

void print_hexdump(char *data, int len) {
	char ascii[16];
	int i, rem;

	for (i = 0; i < len; ) {
		if ((i % 16) == 0) printf("%04x: ", i);

		printf("%02x", (unsigned char)data[i]);
		if (isprint(data[i])) ascii[i%16] = data[i];
		else ascii[i%16] = '.';
		i++;

		if ((i % 2) == 0) printf(" ");
		if ((i % 16) == 0) printf(" %.16s\n", ascii);
	}
	rem = i % 16;
	if (rem) printf("%*.*s\n", 2*(16-rem)+((16-rem+1)/2)+rem+1, rem, ascii);
}

/********** Date **********/
Date::Date(std::string date_str) {
	struct tm timeinfo;
	char buffer[80];
	memset(&timeinfo, 0, sizeof(struct tm));
	strptime(date_str.c_str(), "%Y-%m-%d",  &timeinfo);

	this->date_t = mktime(&timeinfo);
	this->date_str = date_str;
}

Date::Date(time_t date_t) {
	struct tm *timeinfo;
	char buffer[80];

	timeinfo = localtime(&date_t);
	strftime(buffer , 80, "%Y-%m-%d", timeinfo);

	this->date_t = date_t;
	this->date_str = buffer;
}

Date::Date() {
	this->date_t = 0;
	this->date_str = "";
}

std::ostream& operator<<(std::ostream& os, const Date& d) {
	os << d.date_str;
	return os;
}

// bool operator <= (Date const &d1, Date const &d2)  { 
//      return d1.date_t <= d2.date_t;
// } 

void Date::add(int add_days) {
	struct tm *timeinfo;
	char buffer[80];

	date_t += 86400 * add_days;

	timeinfo = localtime(&date_t);
	strftime(buffer , 80, "%Y-%m-%d", timeinfo);
	date_str = buffer;
}

/********** File **********/
File::File(std::string s) {
	std::vector<std::string> folder_sv = split(s, "/");
	std::vector<std::string> fn_sv = split(folder_sv[folder_sv.size()-1], ".");
	this->full_name = s;
	this->fn = fn_sv[0];
	if (fn_sv.size() > 1) this->extension = fn_sv[1];
	for (int i = 0; i < folder_sv.size() - 1; i++) {
		this->path += folder_sv[i] + "/";
	}
}

std::ostream& operator<<(std::ostream& os, const File& f) {
	os << "fn: " << f.fn << std::endl;;
	os << "full_name: " << f.full_name << std::endl;;
	os << "path: " << f.path << std::endl;;
	os << "extension: " << f.extension << std::endl;;
	return os;
}
 
std::vector<std::string> File::get_files_in_dir(const std::string& dir_name) {
	std::vector<std::string> v;
	std::string file_full_name, _dir_name = dir_name;

	if (_dir_name.empty()) return v;

	DIR* dirp = opendir(_dir_name.c_str());
	struct dirent * dp;

	if (_dir_name.back() != '/') _dir_name += "/";

	while ((dp = readdir(dirp)) != NULL) {
		file_full_name = _dir_name + dp->d_name;
		if (File::file_exists(file_full_name))
			v.push_back(file_full_name);
	}
	closedir(dirp);
	return v;
}

bool File::file_exists(const std::string& fn) {
	struct stat info;
	if (stat(fn.c_str(), &info) == 0 && !(info.st_mode & S_IFDIR))
		return true;
	return false;
}

bool File::dir_exists(const std::string& dir_name) {
	struct stat info;
	if (stat(dir_name.c_str(), &info) == 0 && info.st_mode & S_IFDIR)
		return true;
	return false;
}

/********** pcap_file **********/
pcap_file::pcap_file(std::string filename) {
	this->filename = filename;
	this->vaild = true;
	
	// open file
	this->ifs.open(this->filename, std::ios::in);
	if (!this->ifs.good()) {
		this->vaild = false;
		this->error_ss << "open file error " << this->filename << std::endl;;
	}
	
	// read global
	memset(&this->global_hdr, 0, sizeof(struct pcap_global_hdr));
	if (this) {
		if (!this->read_global_header()) {
			this->vaild = false;
			this->error_ss << this->filename << " read_global_header error\n";
		}
	}
}

pcap_file::~pcap_file() {
	// close file
	this->ifs.close();
}

int pcap_file::read(char *buf, int buf_len) {
	int read_len = -1;
	memset(buf, 0, buf_len);

// std::cout << "read in\n";


	// read header
	if (!this->read_record_header()) {
		this->vaild = false;
		this->error_ss << this->filename << " read_record_header error\n";
		goto read_finished;
	}

// std::cout << "read aa\n";
	read_len = MIN((int)this->current_record_hdr.incl_len, buf_len);
// std::cout << "read_len=" << read_len << std::endl;

	// read data
	if (!this->read_record_data(buf, read_len)) {
		this->vaild = false;
		this->error_ss << this->filename << " read_record_data error\n";
		read_len = -1;
		goto read_finished;
	}

if (this->filename == "/data/database/2in1/tcpdump/20210427/TSE_20210427.pcap") {
	std::cout << "read_len=" << read_len << std::endl;
}

// std::cout << "read out\n";

	read_finished:
// std::cout << "read finished\n";
	return read_len;
}

bool pcap_file::read_global_header() {
// std::cout << "read_global_header\n";

	const int LEN = 48;  // 24 bytes
	unsigned char buf[LEN+1];
	int i, c;

// std::cout << "c: ";
	for (i = 0; i < LEN; i=i+2) {
		c = this->ifs.get();
// std::cout << c;
		if (c == EOF) return false;
		snprintf((char*)&buf[i], 2+1, "%02x", c);
	}
// std::cout << std::endl;

	// memcpy(&this->global_hdr, buf, sizeof(buf));  // not check

	return true;
}

bool pcap_file::read_record_header() {
	const int LEN = 32;  // 16 bytes
	unsigned char buf[LEN+1];
	unsigned char c;
	int i;
	char ts_sec[9], ts_usec[9], incl_len[9], orig_len[9];
	char ts_sec_reverse[8], ts_usec_reverse[8], incl_len_reverse[8], orig_len_reverse[8];
	struct tm *timeinfo;

// std::cout << "read_record_header 0\n";
// std::cout << "this->ifs.good()=" << this->ifs.good() << std::endl;

	for (i = 0; i < LEN; i=i+2) {
		c = this->ifs.get();
// std::cout << c << std::endl;
		if (c == EOF) return false;
		snprintf((char*)&buf[i], 2+1, "%02x", c);
	}
// std::cout << "read_record_header 1\n";

	memcpy(ts_sec_reverse, &buf[0], 8);
	memcpy(ts_usec_reverse, &buf[8], 8);
	memcpy(incl_len_reverse, &buf[16], 8);
	memcpy(orig_len_reverse, &buf[24], 8);

	// ts_sec
	memcpy(&ts_sec[0], &ts_sec_reverse[6], 2);
	memcpy(&ts_sec[2], &ts_sec_reverse[4], 2);
	memcpy(&ts_sec[4], &ts_sec_reverse[2], 2);
	memcpy(&ts_sec[6], &ts_sec_reverse[0], 2);
	ts_sec[8] = '\0';
	this->current_record_hdr.ts_sec = strtol(ts_sec, NULL, 16) % 86400;

// std::cout << "read_record_header 2\n";

	//  timeinfo
	// timeinfo = localtime((time_t*)&this->ts_sec);
	// strftime(this->ts_sec_str, sizeof(this->ts_sec_str), "%H:%M:%S", timeinfo);

	// ts_usec
	memcpy(&ts_usec[0], &ts_usec_reverse[6], 2);
	memcpy(&ts_usec[2], &ts_usec_reverse[4], 2);
	memcpy(&ts_usec[4], &ts_usec_reverse[2], 2);
	memcpy(&ts_usec[6], &ts_usec_reverse[0], 2);
	ts_usec[8] = '\0';
	this->current_record_hdr.ts_usec = strtol(ts_usec, NULL, 16);

// std::cout << "read_record_header 3\n";

	// incl_len
	memcpy(&incl_len[0], &incl_len_reverse[6], 2);
	memcpy(&incl_len[2], &incl_len_reverse[4], 2);
	memcpy(&incl_len[4], &incl_len_reverse[2], 2);
	memcpy(&incl_len[6], &incl_len_reverse[0], 2);
	incl_len[8] = '\0';
	this->current_record_hdr.incl_len = strtol(incl_len, NULL, 16);

// std::cout << "this->current_record_hdr.incl_len=" << this->current_record_hdr.incl_len << std::endl;

	// orig_len
	memcpy(&orig_len[0], &orig_len_reverse[6], 2);
	memcpy(&orig_len[2], &orig_len_reverse[4], 2);
	memcpy(&orig_len[4], &orig_len_reverse[2], 2);
	memcpy(&orig_len[6], &orig_len_reverse[0], 2);
	orig_len[8] = '\0';
	this->current_record_hdr.orig_len = strtol(orig_len, NULL, 16);

// std::cout << "read_record_header 5\n";

	if (this->current_record_hdr.incl_len > 1518 || this->current_record_hdr.orig_len  > 1518) return false;

	return true;
}

bool pcap_file::read_record_data(char *buf, int len) {
	unsigned char c;
	long int i;

	for (i = 0; i < len; i++) {
		c = this->ifs.get();
		if (c == EOF) return false;
		buf[i] = c;
	}

	if (errno != 0) return false;

	return true;
}


