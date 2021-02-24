CC=g++

ABSDIR=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
INCLUDES=$(ABSDIR)/..

# CFLAGS=-c -std=c++17 $(addprefix -I, $(INCLUDES))
CFLAGS=-c -std=c++11 -lstdc++fs $(addprefix -I, $(INCLUDES))

LIB=libbsdr.a
LIBOBJS=BSDR.o

all: $(LIB)

$(LIB): $(LIBOBJS)
	ar rcs $@ $^
	rm -f *.o

BSDR.o: BSDR.cpp
	$(CC) $^ $(CFLAGS) -o $@

clean:
	rm -f $(LIB) $(LIBOBJS)

