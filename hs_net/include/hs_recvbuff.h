#ifndef __HS_RECV_BUFF_H__
#define __HS_RECV_BUFF_H__

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "hs_netdef.h"

struct hs_recvbuff {
	char* data_ptr;
	int length;
	int curr_write;
	int curr_read;
};

// 创建接收缓冲
struct hs_recvbuff*
hs_recvbuff_create(int length);

// 销毁接收缓冲
void
hs_recvbuff_destroy(struct hs_recvbuff* hcb);

// 接收缓冲初始化
bool
hs_recvbuff_init(struct hs_recvbuff* hcb, int length);

// 清空接收缓冲
void
hs_recvbuff_clear(struct hs_recvbuff* hcb);

// 接收缓冲剩余的可写字节数
int
hs_recvbuff_leftwrite(struct hs_recvbuff* hcb);

// 用读取套接字数据后写入接收缓冲，返回是否可写入缓冲区
bool
hs_recvbuff_writeto(struct hs_recvbuff* hcb, int fd, int* read_nbytes);

// 接受缓冲剩余可读字节数
int
hs_recvbuff_leftread(struct hs_recvbuff* hcb);

// process_func处理缓冲中的内容
int
hs_recvbuff_data_handle(struct hs_recvbuff* hcb, data_handle handle_func, void* param);

#endif
