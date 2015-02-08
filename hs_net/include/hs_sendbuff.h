#ifndef __HS_SENDBUFF_H__
#define __HS_SENDBUFF_H__

#include <stdbool.h>

enum SendBuffState {
	SENDBUFF_STATE_EMPTY = 0,
	SENDBUFF_STATE_HASDATA = 1,
	SENDBUFF_STATE__FULL = 2,
};

struct hs_sendbuff {
	char* data_ptr;
	int length;
	int write_offset;
	int read_offset;
	int buff_state;
};

// 创建发送缓冲
struct hs_sendbuff*
hs_sendbuff_create(int length);

// 销毁发送缓冲
void
hs_sendbuff_destroy(struct hs_sendbuff* hs);

// 初始化发送缓冲
bool
hs_sendbuff_init(struct hs_sendbuff* hs, int length);

// 反初始化发送缓冲
void
hs_sendbuff_uninit(struct hs_sendbuff* hs);

// 清空缓存
void
hs_sendbuff_clear(struct hs_sendbuff* hs);

// 写入缓冲，要么数据全部写入，要么数据都不写
bool
hs_sendbuff_write(struct hs_sendbuff* hs, const char* data, int datalen);

// 往socket上发送缓冲区数据，该函数需要在主循环中调用，以保证发送缓冲有足够的空间写入数据
int
hs_sendbuff_send(struct hs_sendbuff* hs, int socket);

#endif
