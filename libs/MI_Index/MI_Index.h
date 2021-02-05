#ifndef MI_INDEX_H
#define MI_INDEX_H

#include <iostream>
#include <filesystem>
#include <string>
#include <map>
#include <vector>

namespace fs = std::filesystem;

class MI_Index {
public:
	explicit MI_Index();

	static MI_Index* read_file(fs::directory_entry);

	std::string filename;
	time_t trade_date = 0;

	
private:

};

#endif // MI_INDEX_H