# BSDR_PATH=../BuySellDailyReport/
LIBS_PATH=../../libs

CC=g++
CFLAGS=-O3 -Wall -g -std=c++17  -I$(LIBS_PATH)
DFLAGS=
LDLIBS=-L$(LIBS_PATH)/BSDR -lbsdr -L$(LIBS_PATH)/util -lutil

EXT_CFLAGS=
EXT_LDLIBS=

# LIB=bsdr.a

BIN=tester

all: clean $(BIN)

clean:
	rm -f $(BIN)

tester: tester.cpp
	$(CC) $(CFLAGS) $(DFLAGS) $(EXT_CFLAGS) $(LDLIBS) $(EXT_LDLIBS) -o $@ $^
