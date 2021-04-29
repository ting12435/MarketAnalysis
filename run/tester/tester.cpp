#include <iostream>
#include <map>

#include "BSDR/BSDR.h"
#include "util/util.h"
#include "md_pcap/md_pcap.h"

void test1();
void test2();
void test3();

int main(int argc, char *argv[]) {

	test3();


	return 0;
}

void test1() {
	BSDR::tester();
}

void test2() {
	uint8_t px[5];
	px[0] = 0x12;
	px[1] = 0x34;
	px[2] = 0x56;
	px[3] = 0x78;
	px[4] = 0x90;

	printf("0x%02x%02x%02x%02x%02x\n", px[0], px[1], px[2], px[3], px[4]);
	printf("0x%02x (%d)\n", *(((uint8_t*)px) + 0), *(((uint8_t*)px) + 0));
	// std::cout << *(((uint8_t*)px) + 0) * 10000000 << std::endl;
	std::cout << GET_PX(px) << std::endl;
}

void test3() {
	std::map<std::string, int> m;
	std::map<std::string, int>::iterator iter;

	m["a"] = 1;
	m["b"] = 1;
	m["z"] = 1;

	std::cout << "m.size(): " << m.size() << std::endl;
	if (m.size() > 0) {
		iter = std::prev(m.find("z"));
		std::cout << "m.begin() == m.end(): " << (m.begin() == m.end()) << std::endl;
		std::cout << "iter == m.begin(): " << (iter == m.begin()) << std::endl;
		std::cout << "iter == m.end(): " << (iter == m.end()) << std::endl;
	}
}


