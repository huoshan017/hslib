#ifndef __HS_RINGBHFFER_H__
#define __HS_RINGBUFFER_H__

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

struct hs_ringbuffer {
	char* data_ptr;
	uint32_t length;
	uint32_t curr_read;
	uint32_t curr_write;
	uint8_t is_full;
};

/*************************** ringbuffer *******************************/
void
hs_ringbuffer_init(struct hs_ringbuffer* hr, uint32_t length);

void
hs_ringbuffer_uninit(struct hs_ringbuffer* hr);

struct hs_ringbuffer*
hs_ringbuffer_create(uint32_t length);

void
hs_ringbuffer_destroy(struct hs_ringbuffer* hr);

bool
hs_ringbuffer_can_write(struct hs_ringbuffer* hr, const char* buff, uint32_t bufflen, uint32_t* write_offset);

bool
hs_ringbuffer_write(struct hs_ringbuffer* hr, const char* buff, uint32_t bufflen);

bool
hs_ringbuffer_can_read(struct hs_ringbuffer* hr, uint32_t* buff_offset, uint32_t* buff_len);

bool
hs_ringbuffer_read(struct hs_ringbuffer* hr, char** buff, uint32_t* bufflen);

void
hs_ringbuffer_clear_data(struct hs_ringbuffer* hr);

#endif
