CC=g++

ABSDIR=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
INCLUDES=$(ABSDIR)/..

CFLAGS=-c -std=c++11 $(addprefix -I, $(INCLUDES))

LIB=libmd_pcap.a
LIBOBJS=md_pcap.o

all: clean $(LIB)

$(LIB): $(LIBOBJS)
	ar rcs $@ $^
	rm -f *.o

md_pcap.o: md_pcap.cpp
	$(CC) $^ $(CFLAGS) -o $@

clean:
	rm -f $(LIB) $(LIBOBJS)

