CC=g++

ABSDIR=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
INCLUDES=$(ABSDIR)/..

# CFLAGS=-c -std=c++17 $(addprefix -I, $(INCLUDES))
CFLAGS=-c -std=c++11 -lstdc++fs $(addprefix -I, $(INCLUDES))

LIB=libmi_index.a
LIBOBJS=MI_Index.o

all: $(LIB)

$(LIB): $(LIBOBJS)
	ar rcs $@ $^
	rm -f *.o

MI_Index.o: MI_Index.cpp
	$(CC) $^ $(CFLAGS) -o $@

clean:
	rm -f $(LIB) $(LIBOBJS)

