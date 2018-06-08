#ifndef VARINT_H
#define VARINT_H

#include <stdbool.h>
#include <unistd.h>

bool receive_varint(int fd, size_t * result_ptr) {
	size_t result = 0;
	unsigned int shift = 0;
	while (true) {
		char c;
		if (read(fd, &c, 1) != 1) return false;
		result |= (c & 0x7f) << shift;
		shift += 7;
		if (!(c & 0x80)) break;
	}
	*result_ptr = result;
	return true;
}

bool send_varint(int fd, size_t number) {
	while (true) {
		unsigned char towrite = number & 0x7f;
		number >>= 7;
		if (number != 0) towrite |= 0x80;
		if (write(fd, &towrite, 1) != 1) return false;
		if (number == 0) break;
	}
	return true;
}

#endif
