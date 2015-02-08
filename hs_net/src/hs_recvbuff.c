#include "hs_recvbuff.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <memory.h>
#include <errno.h>
#include <stdio.h>

// 创建接收缓冲
struct hs_recvbuff*
hs_recvbuff_create(int length)
{
	if (!length)
		return NULL;
	
	struct hs_recvbuff* hcb = (struct hs_recvbuff*)malloc(sizeof(struct hs_recvbuff));
	if (!hs_recvbuff_init(hcb, length))
		return NULL;
	
	return hcb;
}

// 销毁接收缓冲
void
hs_recvbuff_destroy(struct hs_recvbuff* hcb)
{
	if (!hcb)
		return;

	free(hcb->data_ptr);
	free(hcb);
}

// 接收缓冲初始化
bool
hs_recvbuff_init(struct hs_recvbuff* hcb, int length)
{
	if (!hcb)
		return false;

	hcb->data_ptr = malloc(length);
	hcb->length = length;
	hcb->curr_write = hcb->curr_read = 0;

	return true;
}

// 清空接收缓冲
void
hs_recvbuff_clear(struct hs_recvbuff* hcb)
{
	hcb->curr_write = hcb->curr_read = 0;
}

// 接收缓冲剩余的可写字节数
int32_t
hs_recvbuff_leftwrite(struct hs_recvbuff* hcb)
{
	if (!hcb) return 0;
	int left = hcb->length - hcb->curr_write;
	if (left <= 0) return 0;
	return (uint32_t)left;
}

// 读取套接字数据后写入接收缓冲，返回值表示是否可写入缓冲区
bool
hs_recvbuff_writeto(struct hs_recvbuff* hcb, int fd, int* read_nbytes)
{
	if (!read_nbytes) return false;
	uint32_t left_write = hs_recvbuff_leftwrite(hcb);
	if (!left_write) return false;
	int n = recv(fd, hcb->data_ptr+hcb->curr_write, left_write, 0);
	if (n < 0) {
		*read_nbytes = -1;
		printf("%s(%d): 接收数据出错(errno:%d)\n", __FILE__, __LINE__, errno);
		return true;
	}

	if (n > 0) {
		hcb->curr_write += n;
		printf("hs_recvbuff_writeto: 从socket(%d)接收了(%d)个字节\n", fd, n);
	} else if (n == 0) {
		printf("hs_recvbuff_writeto: 连接被对方关闭\n");
	}

	*read_nbytes = n;

	return true;
}

// 接受缓冲剩余可读字节数
int32_t
hs_recvbuff_leftread(struct hs_recvbuff* hcb)
{
	if (!hcb) return -1;
	int left = hcb->curr_write - hcb->curr_read;
	return left;
}

// handle_func处理缓冲中的内容
int32_t
hs_recvbuff_data_handle(struct hs_recvbuff* hcb, data_handle handle_func, void* param)
{
	int left_read = hs_recvbuff_leftread(hcb);
	if (left_read < 0)
		return -1;
	else if (left_read == 0)
		return 0;

	int n = handle_func(hcb->data_ptr+hcb->curr_read, left_read, param);
	if (n == 0)
		return 0;
	else if (n <= 0)
		return -1;

	// 剩下的未处理完的挪到最开头
	hcb->curr_read += n;

	left_read = hcb->curr_write - hcb->curr_read;
	if (left_read > 0) {
		memmove(hcb->data_ptr, hcb->data_ptr+hcb->curr_read, left_read);
		printf("hs_recvbuff_data_handle: 有剩余未处理完的数据(%s)\n", (const char*)hcb->data_ptr);
	}
	hcb->curr_read = 0;
	hcb->curr_write = left_read;

	return n;
}
