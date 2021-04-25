#include <map>
#include <cassert>
#include <getopt.h>

#include "md_pcap/md_pcap.h"
#include "util/util.h"

void uplimit();

struct var {
	std::string type;
	Date *d1;
	Date *d2;
} g_var;

void init_g_var() {
	g_var.type = "";
	g_var.d1 = nullptr;
	g_var.d2 = nullptr;
}

bool check_g_var() {
	if (g_var.d1 == nullptr && g_var.d2 == nullptr)
		return false;
	else if (g_var.d1 == nullptr)
		g_var.d1 = g_var.d2;
	else if (g_var.d2 == nullptr)
		g_var.d2 = g_var.d1;
	return true;
}

static struct option long_options[] = {
	{"type", required_argument, NULL, 0},
	{"d1", required_argument, NULL, 1},
	{"d2", required_argument, NULL, 2}
};

int main(int argc, char *argv[]) {

	int c;

	init_g_var();

	if (argc < 2) goto usage_error;

	while ((c = getopt_long (argc, argv, "", long_options, NULL)) != -1) {
		switch (c) {
			case 0:
				g_var.type = optarg;
				break;
			case 1:
				g_var.d1 = new Date(optarg);
				break;
			case 2:
				g_var.d2 = new Date(optarg);
				break;
			default:
				goto usage_error;
		}
	}

	assert(check_g_var());

	if (g_var.type == "uplimit")
		uplimit();

	return 0;

	usage_error:
	fprintf(stderr, "Usage: %s\n", argv[0]);
	fprintf(stderr, "%9s [--type] [--d1] [--d2]\n", " ");
	fprintf(stderr, "  --type: [uplimit]\n");
	fprintf(stderr, "\ne.g.\n");
	fprintf(stderr, "taskset -c 5 %s --type uplimit --d1 2021-04-20 --d2 2021-04-21\n", argv[0]);
	return EXIT_FAILURE;
}

void uplimit() {
	std::map<Date, std::map<std::string, int>> m;  // K:data V:{K:stock V:uplimit_price}
	struct md *frame;

	Date current_date(g_var.d1->date_str);
	// Date end_date(g_var.d2->date_str);
	// while (current_date <= end_date) {
	while (current_date <= *(g_var.d2)) {
	

		// frame = get_pcap_stream(current_date);
		// if (check_md_frame(frame)) {
		// 	if (is_stock(frame)) {
		// 		if (is_trade_uplimit(frame)) {

		// 			std::string feedcode = get_feedcode(frame);

		// 			if (m[current_date].find(feedcode) == m[current_date].end()) {

		// 				struct md_px_lt *trade_pxlt = get_trade_pxlt(frame);
		// 				m[current_date][feedcode] = get_px(trade_pxlt);

		// 			}
		// 		}
		// 	}
		// }

		current_date.add(1);
	}

}



