CC=g++
# CFLAGS=-c -std=c++17
CFLAGS=-c -std=c++11

LIB=libutil.a
LIBOBJS=util.o

all: $(LIB)

$(LIB): $(LIBOBJS)
	ar rcs $@ $^
	rm -f *.o

util.o: util.cpp
	$(CC) $^ $(CFLAGS) -o $@

clean:
	rm -f $(LIB) $(LIBOBJS)

