#ifndef PBLOCK_H
#define PBLOCK_H

/**
 * Pblock v0.2 protocol routines implementation.
 *
 * The implementation is quite unstructural (eg. reqires caller to take
 * care of writing and reading data to stream). This is tradeof for
 * efficiency and versatility. For example you can use this library with
 * `splice` syscalls to leverage zero-copy data handling.
 */

#include <stdbool.h>
#include <unistd.h>
#include <varint.h>
#include <err.h>
#include <sys/socket.h>
#include <stdio.h>

#ifndef PBLOCK_DEBUG
	#define PBLOCK_DEBUG false
#endif

typedef char segment_type_t;

/* Header types */
#define PBLOCK_SEGMENT_TYPE_READ ((segment_type_t)'r')
#define PBLOCK_SEGMENT_TYPE_WRITE ((segment_type_t)'w')
#define PBLOCK_SEGMENT_TYPE_COMMIT ((segment_type_t)'c')

struct pblock_segment_header_t {
	segment_type_t type;
	size_t offset;
	size_t size;
};

/**
 * Segment header is received from `fd` and stored under `segment` pointer.
 */
bool pblock_receive_segment_header(int fd, struct pblock_segment_header_t * segment) {
	if (PBLOCK_DEBUG) fprintf(stderr, "pblock %d: receiving header\n", fd);

	if (read(fd, &segment->type, 1) != 1) {
		warnx("failed to receive segment type");
		return false;
	}
	switch (segment->type) {
		case (PBLOCK_SEGMENT_TYPE_READ):
		case PBLOCK_SEGMENT_TYPE_WRITE:
			if (!receive_varint(fd, &segment->offset)) {
				warnx("failed to receive segment offset");
				return false;
			}
			if (!receive_varint(fd, &segment->size)) {
				warnx("failed to receive segment size");
				return false;
			}
			break;
		case PBLOCK_SEGMENT_TYPE_COMMIT:
			break;
		default:
			warnx("received unknown segment type '%c'", segment->type);
			return false;
	}

	if (PBLOCK_DEBUG) fprintf(
		stderr,
		"pblock %d: received header '%c'\n",
		fd, segment->type
	);
	return true;
}

bool pblock_send_segment_header(int fd, struct pblock_segment_header_t * segment) {
	if (PBLOCK_DEBUG) fprintf(stderr, "pblock %d: sending header '%c'\n", fd, segment->type);

	if (write(fd, &segment->type, 1) != 1) {
		warn("failed to send segment type");
		return false;
	}
	switch (segment->type) {
		case PBLOCK_SEGMENT_TYPE_READ:
		case PBLOCK_SEGMENT_TYPE_WRITE:
			if (!send_varint(fd, segment->offset)) {
				warn("failed to send segment offset");
				return false;
			}
			if (!send_varint(fd, segment->size)) {
				warn("failed to send segment size");
				return false;
			}
			break;
		case PBLOCK_SEGMENT_TYPE_COMMIT:
			break;
		default:
			warn("requested to send unknown header type");
			return false;
	}
	return true;
}

/**
 * Send read request, then receive and check incoming response.
 * After calling this function exactly `size` bytes must be read from `rfd`.
 */
bool pblock_request_data(int wfd, int rfd, size_t offset, size_t size) {
	struct pblock_segment_header_t header;

	header.type = PBLOCK_SEGMENT_TYPE_READ;
	header.offset = offset;
	header.size = size;
	if (!pblock_send_segment_header(wfd, &header)) {
		warnx("failed to request data");
		return false;
	}

	if (!pblock_receive_segment_header(rfd, &header)) {
		warnx("failed to receive response header");
		return false;
	}
	if (header.type != PBLOCK_SEGMENT_TYPE_WRITE) {
		warnx("wrong response header type");
		return false;
	}
	if (header.offset != offset) {
		warnx("wrong response header offset");
		return false;
	}
	if (header.size != size) {
		warnx("wrong response header size");
		return false;
	}

	return true;
}

/**
 * Send write header.
 * After calling this function exactly `size` bytes must be written to `wfd`.
 */
bool pblock_request_write(int wfd, size_t offset, size_t size) {
	struct pblock_segment_header_t header;
	header.type = PBLOCK_SEGMENT_TYPE_WRITE;
	header.offset = offset;
	header.size = size;
	return pblock_send_segment_header(wfd, &header);
}

bool pblock_exchange_commits(int wfd, int rfd) {
	struct pblock_segment_header_t header;
	header.type = PBLOCK_SEGMENT_TYPE_COMMIT;
	return (
		pblock_send_segment_header(wfd, &header) &&
		pblock_receive_segment_header(rfd, &header) &&
		header.type == PBLOCK_SEGMENT_TYPE_COMMIT
	);
}

void pblock_terminate_session(int wfd, int rfd) {
	if (shutdown(wfd, SHUT_WR) != 0) {
		warn("problem during closing writing stream");
	}
	char ch;
	if (read(rfd, &ch, 1) != 0) {
		warnx(
			"Reading stream wasn't empty on session termination. "
			"Either server is misbehaving or desynchronization ocurred."
		);
	}
	if (shutdown(rfd, SHUT_RD) != 0) {
		warn("problem during closing reading stream");
	}
}

#endif
