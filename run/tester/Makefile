LIBS_PATH=../../libs

CC=g++
CFLAGS=-O3 -Wall -g -std=c++11  -I$(LIBS_PATH)
DFLAGS=
LDLIBS=-L/home/tim/MarketAnalysis/libs/BSDR -lbsdr -L/home/tim/MarketAnalysis/libs/util -lutil

EXT_CFLAGS=
EXT_LDLIBS=

BIN=tester

all: clean make_lib $(BIN)

make_lib:
	make -C $(LIBS_PATH)/md_pcap
	make -C $(LIBS_PATH)/util

clean:
	rm -f $(BIN)

tester: tester.cpp
	$(CC) $^ $(CFLAGS) $(DFLAGS) $(EXT_CFLAGS) $(LDLIBS) $(EXT_LDLIBS) -o $@
