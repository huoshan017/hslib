#include "hs_continuousbuffer.h"

struct hs_continuousbuffer*
hs_continuousbuffer_create(uint32_t length)
{
	if (length == 0)
		return NULL;

	struct hs_continuousbuffer* hb = (struct hs_continuousbuffer*)malloc(sizeof(struct hs_continuousbuffer));
	bool res = hs_continuousbuffer_init(hb);
	if (!res) {
		free(hb);
		return NULL;
	}
	return hb;
}

void
hs_continuousbuffer_destroy(struct hs_continuousbuffer* hb)
{
	if (!hb)
		return;

	free(hb->data_ptr);
	free(hb);
}

bool
hs_continuousbuffer_init(struct hs_continuousbuffer* hb)
{
	if (!hb)
		return false;
	
	hb->data_ptr = (char*)malloc(length*sizeof(char));
	hb->length = length;
	hb->curr_write = hb->curr_read = 0;
	return true;
}

void
hs_continuousbuffer_clear(struct hs_continuousbuffer* hb)
{
	if (hb) {
		hb->curr_write = 0;
		hb->curr_read = 0;
	}
}

// func为数据读取函数，读到数据后写入continuousbuffer，返回值是写入的字节数，-1表示读取出错
int
hs_continuousbuffer_write(struct hs_continuousbuffer* hb, data_from_func func)
{
}

uint32_t
hs_continuousbuffer_leftwrite(struct hs_continuousbuffer* hb)
{
}

bool
hs_continuousbuffer_writelen(struct hs_continuousbuffer* hb, uint32_t writelen)
{
}

char*
hs_continuousbuffer_readptr(struct hs_continuousbuffer* hb)
{
}

uint32_t
hs_continuousbuffer_leftread(struct hs_continuousbuffer* hb)
{
}

bool
hs_continuousbuffer_readlen(struct hs_continuousbuffer* hb, uint32_t readlen)
{
}
