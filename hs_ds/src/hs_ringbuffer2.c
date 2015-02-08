#include "hs_ringbuffer2.h"
#include <stdlib.h>
#include <stdio.h>

struct hs_ringbuffer2*
hs_ringbuffer2_create(uint32_t length)
{
	if (!length)
		return NULL;

	struct hs_ringbuffer2* hr = (struct hs_ringbuffer2*)malloc(sizeof(struct hs_ringbuffer2));
	if (!hs_ringbuffer2_init(hr)) {
		free(hr);
		return NULL;
	}
	return hr;
}

void
hs_ringbuffer2_destroy(struct hs_ringbuffer2* rb)
{
	if (rb) {
		if (rb->data_ptr) {
			free(rb->data_ptr);
			rb->data_ptr = NULL;
		}
		free(rb);
	}
}

bool
hs_ringbuffer2_init(struct hs_ringbuffer2* rb, uint32_t length)
{
	if (!rb)
		return false;

	rb->data_ptr = malloc(sizeof(char)*length);
	rb->length = length;
	rb->write_offset = 0;
	rb->read_offset = 0;

	return true;
}

void
hs_ringbuffer2_uninit(struct hs_ringbuffer2* rb)
{
	if (rb) {
		if (rb->data_ptr) {
			free(rb->data_ptr);
		}
		rb->length = 0;
		rb->write_offset = 0;
		rb->read_offset = 0;
	}
}

bool
hs_ringbuffer2_canwrite(struct hs_ringbuffer2* rb, uint32_t datalen)
{
	if (!rb)
		return false;

	int left = 0;
	if (rb->write_offset > rb->read_offset) {
		left = rb->write_offset - rb->read_offset;
	} else if (rb->write_offset < rb->read_offset) {
		left = rb->length - rb->read_offset + rb->write_offset;
	} else {
		
	}

	return true;
}

bool
hs_ringbuffer2_write(struct hs_ringbuffer2* rb, const char* data, uint32_t datalen)
{
}

bool
hs_ringbuffer2_canread(struct hs_ringbuffer2* rb)
{
}

bool
hs_ringbuffer2_read(struct hs_ringbuffer2* rb, char* data, uint32_t datalen)
{
}

void
hs_ringbuffer2_cleardata(struct hs_ringbuffer2* rb)
{
}
