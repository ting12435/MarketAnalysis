/**
 * \file
 * \brief Miscellaneous utility functions.
 */
#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <vector>
#include <initializer_list>
#include <fstream>
#include <sstream>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define OUTPUT(msg) std::cout << msg << std::endl;

class Date {
public:
	explicit Date(std::string date_str);  // 2021-01-29
	explicit Date(time_t date_t);
	explicit Date();

	void add(int add_days);

	friend std::ostream& operator<<(std::ostream&, const Date&);
	friend bool operator == (Date const &d1, Date const &d2) { return d1.date_t == d2.date_t; } 
	friend bool operator <= (Date const &d1, Date const &d2) { return d1.date_t <= d2.date_t; }
	friend bool operator >= (Date const &d1, Date const &d2) { return d1.date_t >= d2.date_t; }
	friend bool operator < (Date const &d1, Date const &d2) { return d1.date_t < d2.date_t; }
	friend bool operator > (Date const &d1, Date const &d2) { return d1.date_t > d2.date_t; }

	std::string date_str;
	time_t date_t;
};

class File {
public:
	explicit File();
	explicit File(std::string);

	friend std::ostream& operator<<(std::ostream&, const File&);

	static std::vector<std::string> get_files_in_dir(const std::string& dir_name);
	static bool file_exists(const std::string& fn);
	static bool dir_exists(const std::string& dir_name);

	std::string fn;
	std::string full_name;
	std::string path;
	std::string extension;
};

class pcap_file {
public:
	pcap_file(std::string filename);
	~pcap_file();

	int read(char *buf, int buf_len);

	bool eof() { return this->ifs.eof(); }

	std::string get_error() { return this->error_ss.str(); }

	operator bool() const { return this->vaild; }

	struct pcap_global_hdr {
		uint32_t magic_number;   /* magic number (0xa1b2c3d4 or 0xd4c3b2a1) */
		uint16_t version_major;  /* major version number */
		uint16_t version_minor;  /* minor version number */
		int32_t  thiszone;       /* GMT to local correction */
		uint32_t sigfigs;        /* accuracy of timestamps */
		uint32_t snaplen;        /* max length of captured packets, in octets */
		uint32_t network;        /* data link type */
	} global_hdr;

	struct pcap_record_hdr {
		uint32_t ts_sec;         /* timestamp seconds */
		uint32_t ts_usec;        /* timestamp microseconds */
		uint32_t incl_len;       /* number of octets of packet saved in file */
		uint32_t orig_len;       /* actual length of packet */
	} current_record_hdr;

	std::ifstream ifs;
	std::string filename;

private:
	bool read_global_header();
	bool read_record_header();
	bool read_record_data(char *buf, int len);

	bool vaild;
	std::stringstream error_ss;
};

std::vector<std::string> split(const std::string& s, std::string delim = ",", int split_cnt = -1);

std::string only_number_and_str(const std::string& s);

void output_csv(std::ostream& os, std::initializer_list<std::string> args);
void output_delimiter_str(std::ostream& os, std::string delimiter, std::initializer_list<std::string> args);



#endif /* UTIL_H */

