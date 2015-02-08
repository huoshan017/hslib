#ifndef __HS_CONTINUOUSBUFFER_H__
#define __HS_CONTINUOUSBUFFER_H__

#include <stdlib.h>

struct hs_continuousbuffer {
	char* data_ptr;
	uint32_t length;
	uint32_t curr_read;
	uint32_t curr_write;
};

typedef int (*data_from_func)(int fd, char* buf, int len);

struct hs_continuousbuffer*
hs_continuousbuffer_create(uint32_t length);

void
hs_continuousbuffer_destroy(struct hs_continuousbuffer* hb);

void
hs_continuousbuffer_clear(struct hs_continuousbuffer* hb);

int
hs_continuousbuffer_write(struct hs_continuousbuffer* hb, data_from_func func);

uint32_t
hs_continuousbuffer_leftwrite(struct hs_continuousbuffer* hb);

char*
hs_continuousbuffer_readptr(struct hs_continuousbuffer* hb);

uint32_t
hs_continuousbuffer_leftread(struct hs_continuousbuffer* hb);

bool
hs_continuousbuffer_readlen(struct hs_continuousbuffer* hb, uint32_t readlen);

#endif
