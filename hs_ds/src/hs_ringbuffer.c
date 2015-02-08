#include "hs_ringbuffer.h"
#include <memory.h>
#include <stdio.h>

// 如果写偏移curr_write和读偏移curr_read相等，这样无法区分是写满了还是读完了
// 用标记is_full表示是否已满

static const int s_reserved_length = 1;

void
hs_ringbuffer_init(struct hs_ringbuffer* hr, uint32_t length)
{
	if (!hr || !length) return;
	hr->data_ptr = malloc(length);
	hr->length = length;
	hr->curr_read = hr->curr_write = 0;
	hr->is_full = 0;
}

void
hs_ringbuffer_uninit(struct hs_ringbuffer* hr)
{
	if (!hr) return;
	free(hr->data_ptr);
	hr->data_ptr = NULL;
	hr->length = 0;
	hr->curr_read = hr->curr_write = 0;
	hr->is_full = 0;
}

struct hs_ringbuffer*
hs_ringbuffer_create(uint32_t length)
{
	if (!length)
		return NULL;

	struct hs_ringbuffer* hr = (struct hs_ringbuffer*)malloc(sizeof(struct hs_ringbuffer));
	hs_ringbuffer_init(hr, length);
	return hr;
}

void
hs_ringbuffer_destroy(struct hs_ringbuffer* hr)
{
	if (hr) {
		hs_ringbuffer_uninit(hr);
		free(hr);
	}
}

// 检查是否有一段连续的缓存可用
bool
hs_ringbuffer_can_write(struct hs_ringbuffer* hr, const char* buff, uint32_t bufflen, uint32_t* write_offset)
{
	if (!hr)
		return false;

	if (hr->is_full)
		return false;

	uint32_t need_len = bufflen + 2;
	int left = 0;

	if (hr->curr_write >= hr->curr_read) {
		left = hr->length - 1 - hr->curr_write;
		// 剩下的长度不够，需要回到缓冲区的开头。
		// 如果剩下的长度不为0的话，会把下一个字节设成0xff，告诉读操作后面的数据需要从缓冲区的头部继续读
		if (left < need_len) {
			left = hr->curr_read;
			if (write_offset)
				*write_offset = 0;
			if (left > 0) {
				// 用0xff是因为下一段数据的第一个字节不可能是0xff，除非数据长度超过64k，而这种情况是坚决要避免的，我们一般会把包长限制在8k以内
				hr->data_ptr[hr->curr_write] = 0xff;
			}
		} else {
			if (write_offset) {
				*write_offset = hr->curr_write;
			}
		}
	} else {
		left = hr->curr_read - hr->curr_write;
		if (write_offset)
			*write_offset = hr->curr_write;
	}
	
	if (left < need_len)
		return false;

	return true;
}

// 写入一段连续的缓存
bool
hs_ringbuffer_write(struct hs_ringbuffer* hr, const char* buff, uint32_t bufflen)
{
	uint32_t write_offset = 0;
	if (!hs_ringbuffer_can_write(hr, buff, bufflen, &write_offset))
		return false;

	hr->data_ptr[write_offset] = (bufflen >> 8) & 0xff;
	hr->data_ptr[write_offset+1] = bufflen & 0xff;

	memcpy(&hr->data_ptr[write_offset+2], buff, bufflen);

	hr->curr_write = write_offset + (2 + bufflen);
	printf("hs_ringbuffer_write: hr->curr_write = %d\n", hr->curr_write);
	if (hr->curr_write == hr->curr_read) {
		printf("hs_ringbuffer_write: write buffer %s full", buff);
		hr->is_full = 1;
	}

	return true;
}

bool
hs_ringbuffer_can_read(struct hs_ringbuffer* hr, uint32_t* buff_offset, uint32_t* buff_len)
{
	if (!hr)
		return false;

	if (hr->curr_read == hr->curr_write) {
		if (!hr->is_full) {
			printf("hs_ringbuffer_can_read: buffer is empty, curr_read=%d, curr_write=%d\n", hr->curr_read, hr->curr_write);
			return false;
		}
	}

	int l = 0;
	uint32_t offset = 0;
	if (hr->curr_read >= hr->curr_write) {
		l = hr->length - 1 - hr->curr_read;
		if (l < 2) {
			offset = 0;
			l = hr->curr_write;
			printf("hs_ringbuffer_can_read: l(%d)<2\n", l);
		} else {
			// 表示后面没数据了
			if ((unsigned char)(hr->data_ptr[hr->curr_read]) == 0xff) {
				offset = 0;
				l = hr->curr_write;
				printf("hs_ringbuffer_can_read: hr->data_ptr[%d]==0xff", hr->curr_read);
			} else {
				offset = hr->curr_read;
				printf("hs_ringbuffer_can_read: hr->data_ptr[%d]=%d\n", hr->curr_read, hr->data_ptr[hr->curr_read]);
			}
		}
	} else if (hr->curr_read < hr->curr_write) {
		l = hr->curr_write - hr->curr_read;
		if (l < 2) {
			printf("hs_ringbuffer_can_read: data_len(%d) error. curr_write(%d), curr_read(%d)\n", l-2, hr->curr_write, hr->curr_read);
			return false;
		}

		offset = hr->curr_read;
	}

	uint32_t data_len = ((hr->data_ptr[offset]>>8)&0xff) + (hr->data_ptr[offset+1]&0xff);
	if (data_len > l-2) {
		printf("hs_ringbuffer_can_read: data_len(%d) > can_read_len(%d)\n", data_len, l-2);
		return false;
	}

	if (buff_offset)
		*buff_offset = offset + 2;

	if (buff_len)
		*buff_len = data_len;

	return true;
}

bool
hs_ringbuffer_read(struct hs_ringbuffer* hr, char** buff, uint32_t* bufflen)
{
	uint32_t read_offset = 0;
	uint32_t read_len = 0;
	if (!hs_ringbuffer_can_read(hr, &read_offset, &read_len))
		return false;

	// 移动读指针
	hr->curr_read = read_offset + read_len;

	if (hr->is_full)
		hr->is_full = 0;

	if (buff) {
		*buff = hr->data_ptr + read_offset;
	}

	if (bufflen) {
		*bufflen = read_len;
	}

	return true;
}

void
hs_ringbuffer_clear_data(struct hs_ringbuffer* hr)
{
	if (!hr) return;
	hr->curr_write = hr->curr_read = 0;
	hr->is_full = 0;
}
