#ifndef __HS_RINGBUFFER2_H__
#define __HS_RINGBUFFER2_H__

#include <stdint.h>

struct hs_ringbuffer2 {
	char* data_ptr;
	uint32_t length;
	uint32_t write_offset;
	uint32_t read_offset;
};

struct hs_ringbuffer2*
hs_ringbuffer2_create(uint32_t length);

void
hs_ringbuffer2_destroy(struct hs_ringbuffer2* rb);

bool
hs_ringbuffer2_init(struct hs_ringbuffer2* rb, uint32_t length);

void
hs_ringbuffer2_uninit(struct hs_ringbuffer2* rb);

bool
hs_ringbuffer2_canwrite(struct hs_ringbuffer2* rb, uint32_t datalen);

bool
hs_ringbuffer2_write(struct hs_ringbuffer2* rb, const char* data, uint32_t datalen);

bool
hs_ringbuffer2_canread(struct hs_ringbuffer2* rb);

bool
hs_ringbuffer2_read(struct hs_ringbuffer2* rb, char* data, uint32_t datalen);

void
hs_ringbuffer2_cleardata(struct hs_ringbuffer2* rb);

#endif
