LIBS_PATH=../../libs

CC=g++
CFLAGS=-O3 -Wall -g -std=c++11 -I$(LIBS_PATH)
DFLAGS=
LDLIBS=-L$(LIBS_PATH)/md_pcap -lmd_pcap -L$(LIBS_PATH)/util -lutil

EXT_CFLAGS=
EXT_LDLIBS=

BIN=main

all: clean make_lib $(BIN)

make_lib:
	make -C $(LIBS_PATH)/md_pcap $(LIBS_PATH)/util

clean:
	rm -f $(BIN)

main: main.cpp
	$(CC) $^ $(CFLAGS) $(DFLAGS) $(EXT_CFLAGS) $(LDLIBS) $(EXT_LDLIBS) -o $@
