CFLAGS = -Wall -Wextra --pedantic -I pblock-c -I buse
HEADERS = buse/buse.h pblock-c/pblock.h pblock-c/varint.h pblock-c/fd_util.h
SOURCES = buse/buse.c pblock_bd.c

.PHONY: all test clean

all: pblock-bd

pblock-bd: $(HEADERS) $(SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SOURCES)

test: pblock-bd
	./test.sh

clean:
	rm -f pblock-bd
