#ifndef UTIL_H
#define UTIL_H

#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>

size_t read_exact(int fd, void * buf, size_t size) {
	size_t offset = 0;
	while (size > offset) {
		ssize_t read_result = read(fd, (char *)buf + offset, size);
		if (read_result <= 0) return offset;
		offset += read_result;
	}
	return offset;
}

bool move_between_fds(int fd_in, int fd_out, size_t size) {
	while (size > 0) {
		ssize_t splice_result = splice(fd_in, NULL, fd_out, NULL, size, 0);
		if (splice_result <= 0) return false;
		size -= splice_result;
	}
	return true;
}

#endif
