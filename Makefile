CFLAGS = -Wall -Wextra --pedantic -I pblock-c -I buse
HEADERS = buse/buse.h pblock-c/pblock.h pblock-c/varint.h pblock-c/fd_util.h
SOURCES = buse/buse.c pblock_buse.c

.PHONY: all test clean

all: pblock-buse

pblock-buse: $(HEADERS) $(SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SOURCES)

test: pblock-buse
	./test.sh

clean:
	rm -f pblock-buse
