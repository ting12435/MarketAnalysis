/**
 * \file
 * \brief Miscellaneous utility functions.
 */
#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <vector>
#include <initializer_list>

#define OUTPUT(msg) std::cout << msg << std::endl;

class Date {
public:
	explicit Date(std::string date_str);  // 2021-01-29
	explicit Date(time_t date_t);
	explicit Date();

	void add(int add_days);

	friend std::ostream& operator<<(std::ostream&, const Date&);
	friend bool operator <= (Date const &, Date const &); 

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

std::vector<std::string> split(const std::string& s, std::string delim = ",", int split_cnt = -1);

std::string only_number_and_str(const std::string& s);

void output_csv(std::ostream& os, std::initializer_list<std::string> args);
void output_delimiter_str(std::ostream& os, std::string delimiter, std::initializer_list<std::string> args);



#endif /* UTIL_H */

