#include "util.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>

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





