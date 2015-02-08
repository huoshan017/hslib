#include "../include/hs_ringbuffer.h"
#include <string.h>
#include <stdio.h>
#include <memory.h>

int main(int argc, char** argv) {
	static int buffer_length = 1024;
	static const char* buffers[] = {
		"worinima",
		"worinimama",
		"123456789012345678901234567890",
		"我日你妈妈",
		"坚持每天打飞机"
	};
	struct hs_ringbuffer* hr = hs_ringbuffer_create(buffer_length);

	while (1) {
		size_t i = 0;
		size_t s = sizeof(buffers)/sizeof(buffers[0]);
		for (; i<s; ++i) {
			hs_ringbuffer_write(hr, buffers[i], strlen(buffers[i]));
		}

		char* buf = NULL;
		uint32_t buf_len = 0;
		bool res = false;
		char tmp[1024];
		while (1) {
			res = hs_ringbuffer_read(hr, &buf, &buf_len);
			if (!res) {
				break;
			}

			memcpy(tmp, buf, buf_len);
			tmp[buf_len] = '\0';
			printf("buffers: %s\n", tmp);
		}
	}

	hs_ringbuffer_destroy(hr);

	return 0;
}
