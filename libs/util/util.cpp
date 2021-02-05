#include "util.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

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

std::ostream& operator<<(std::ostream& os, const Date& d) {
	os << d.date_str;
	return os;
}

bool operator <= (Date const &d1, Date const &d2) 
{ 
     return d1.date_t <= d2.date_t;
} 

void Date::add(int add_days) {
	struct tm *timeinfo;
	char buffer[80];

	date_t += 86400 * add_days;

	timeinfo = localtime(&date_t);
	strftime(buffer , 80, "%Y-%m-%d", timeinfo);
	date_str = buffer;
}

void split(const std::string& s, std::vector<std::string>& sv, std::string delim, int split_cnt) {
    int i;
    std::string temp;
    std::istringstream iss(s);
    sv.clear();

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