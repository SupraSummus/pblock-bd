CFLAGS = -Wall -Wextra --pedantic -I pblock-c -I buse
HEADERS = buse/buse.h pblock-c/pblock.h pblock-c/varint.h pblock-c/fd_util.h
SOURCES = buse/buse.c pblock_bd.c

# where to install
DESTDIR       =
PREFIX        = /usr/local
INSTALL       = /usr/bin/env install
bindir        = $(PREFIX)/bin

.PHONY: all test install clean

all: pblock-bd

pblock-bd: $(HEADERS) $(SOURCES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SOURCES)

test: pblock-bd
	./test.sh

install: pblock-bd
	$(INSTALL) -d $(DESTDIR)$(bindir)
	$(INSTALL) -m755 $^ $(DESTDIR)$(bindir)/$^

clean:
	rm -f pblock-bd
