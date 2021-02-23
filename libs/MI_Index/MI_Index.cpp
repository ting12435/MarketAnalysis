#include "MI_Index.h"

#define MI_INDEX_FOLDER "../../data/MI_Index/MI_INDEX_ALL/"

mi_index_data_t MI_Index::get_data(Date st_date, Date ed_date) {

	mi_index_data_t d;
	fs::path dir{MI_INDEX_FOLDER};

	if (fs::exists(dir)) {

		Date current_date(st_date.date_str);
		while (current_date <= ed_date) {

			current_date.add(1);
		}
	}

	return d;
}

MI_Index_Data* MI_Index_Data::read_file(fs::directory_entry file) {
	return NULL;
}