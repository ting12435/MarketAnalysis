#include <iostream>

#include "BSDR/BSDR.h"
#include "util/util.h"
#include "md_pcap/md_pcap.h"

void test1();
void test2();

int main(int argc, char *argv[]) {

	test2();


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
	std::cout << GET_PX(px) << std::endl;
}




