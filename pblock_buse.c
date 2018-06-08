#define _GNU_SOURCE
#include <buse.h>
#include <pblock.h>
#include <fd_util.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>

struct pblock_connection {
	int rfd;
	int wfd;
};

static int pblock_read(void *buf, u_int32_t len, u_int64_t offset, void *userdata) {
	struct pblock_connection * conn = userdata;
	if (
		!pblock_request_data(conn->wfd, conn->rfd, offset, len) ||
		read_exact(conn->rfd, buf, len) != len ||
		!pblock_exchange_commits(conn->wfd, conn->rfd)
	) {
		return -1;
	} else {
		return 0;
	}
}

static int pblock_write(const void *buf, u_int32_t len, u_int64_t offset, void *userdata) {
	struct pblock_connection * conn = userdata;
	if (
		!pblock_request_write(conn->wfd, offset, len) ||
		write(conn->wfd, buf, len) != len ||
		!pblock_exchange_commits(conn->wfd, conn->rfd)
	) {
		return -1;
	} else {
		return 0;
	}
}

static void pblock_disconnect(void *userdata) {
	struct pblock_connection * conn = userdata;
	pblock_terminate_session(conn->wfd, conn->rfd);
}

int main(int argc, char * * argv) {
	/* cmd line sanity checking */
	if (argc != 4) {
		fprintf(
			stderr, 
			"Usage: %s <pblock fd> <size of block device> /dev/nbd0\n",
			argv[0]
		);
		return EXIT_FAILURE;
	}
	char * ch_end;
	int fd = strtol(argv[1], &ch_end, 0);
	if (*ch_end != '\0' || fd < 0) {
		errx(EXIT_FAILURE, "pblock fd must be nonegative integer");
	}
	long long size = strtoll(argv[2], &ch_end, 0);
	if (*ch_end != '\0' || size < 0) {
		errx(EXIT_FAILURE, "device size must be nonegative integer");
	}

	/* construct operations and run buse */
	fprintf(stderr, "starting pblock-buse\n");
	struct buse_operations operations = {
		.read = pblock_read,
		.write = pblock_write,
		.disc = pblock_disconnect,
		.flush = NULL,
		.trim = NULL,
		.size = size,
	};

	struct pblock_connection pblock = {
		.rfd = fd,
		.wfd = fd,
	};

	return buse_main(argv[3], &operations, &pblock);
}
