#include "hs_sendbuff.h"
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <errno.h>

// 创建发送缓冲
struct hs_sendbuff*
hs_sendbuff_create(int length)
{
	struct hs_sendbuff* hs = (struct hs_sendbuff*)malloc(sizeof(struct hs_sendbuff));
	if (!hs_sendbuff_init(hs, length))
		return NULL;
	return  hs;
}

// 销毁发送缓冲
void
hs_sendbuff_destroy(struct hs_sendbuff* hs)
{
	if (hs) {
		hs_sendbuff_uninit(hs);
		free(hs);
	}
}

// 初始化发送缓冲
bool
hs_sendbuff_init(struct hs_sendbuff* hs, int length)
{
	if (!hs || !length)
		return false;

	hs->data_ptr = malloc(sizeof(char)*length);
	hs->length = length;
	hs->write_offset = 0;
	// -1表示没有数据可读
	hs->read_offset = 0;
	hs->buff_state = SENDBUFF_STATE_EMPTY;

	return true;
}

// 反初始化发送缓冲
void
hs_sendbuff_uninit(struct hs_sendbuff* hs)
{
	if (hs) {
		if (hs->data_ptr) {
			free(hs->data_ptr);
		}
		hs->length = 0;
		hs->write_offset = 0;
		hs->read_offset = 0;
		hs->buff_state = SENDBUFF_STATE_EMPTY;
	}
}

// 清空缓存
void
hs_sendbuff_clear(struct hs_sendbuff* hs)
{
	if (hs) {
		hs->write_offset = 0;
		hs->read_offset = 0;
	}
}

// 写入缓冲（写入一次缓冲为连续的，长度不够不会写回开头）
bool
hs_sendbuff_write(struct hs_sendbuff* hs, const char* data, int datalen)
{
	if (!hs || !data || datalen==0)
		return false;

	if (hs->length == 0)
		return false;

	int canwrite = 0;
	
	// 空缓冲区可写字节数为缓冲区大小
	if (hs->write_offset > hs->read_offset) {
		canwrite = hs->length - hs->write_offset;
	} else {
		if (hs->write_offset == 0) {
			canwrite = hs->length;
		} else {
			printf("hs_sendbuff_write: 写偏移(%d)小于等于读偏移(%d)，error!!!\n", hs->write_offset, hs->read_offset);
			return false;
		}
	}

	if (canwrite < datalen) {
		printf("hs_sendbuff_write: 可写字节数(%d) < 数据长度(%d)\n", canwrite, datalen);
		return false;
	}

	memcpy(hs->data_ptr+hs->write_offset, data, datalen);
	hs->write_offset += datalen;

	return true;
}

// 往socket上发送缓冲区数据
int
hs_sendbuff_send(struct hs_sendbuff* hs, int socket)
{
	if (!hs)
		return -1;

	int cansend = hs->write_offset - hs->read_offset;
	if (cansend <= 0) {
		return 0;
	}

	int s = send(socket, hs->data_ptr + hs->read_offset, cansend);
	if (s < 0) {
		printf("hs_sendbuff_send: 发送缓冲区数据失败, errno(%d)\n", errno);
		return -1;
	}

	hs->read_offset += s;
	if (hs->read_offset == hs->write_offset) {
		hs->read_offset = hs->write_offset = 0;
	}

	return s;
}
