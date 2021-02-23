CC=g++

ABSDIR=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
INCLUDES=$(ABSDIR)/..

CFLAGS=-c -std=c++17 $(addprefix -I, $(INCLUDES))

LIB=mi_index.a
LIBOBJS=mi_index.o

all: $(LIB)

$(LIB): $(LIBOBJS)
	ar rcs $@ $^
	rm -f *.o

mi_index.o: mi_index.cpp
	c++ $(CFLAGS) $^ -o $@

clean:
	rm -f $(LIB) $(LIBOBJS)

