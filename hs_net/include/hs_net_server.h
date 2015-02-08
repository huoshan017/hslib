#ifndef __HS_SERVER_H__
#define __HS_SERVER_H__

#include <stdint.h>
#include "hs_netdef.h"

// 新建服务器
struct hs_net_server*
hs_net_server_create(short port, uint32_t max_agent_size);

// 销毁服务器
void
hs_net_server_destroy(struct hs_net_server* server);

// 关闭服务器
void
hs_net_server_close(struct hs_net_server* server);

// 等待事件，返回能读取的agent个数
int
hs_net_server_wait(struct hs_net_server* server);

// 处理事件
int
hs_net_server_process_event(struct hs_net_server* server, int index);

// 接受新连接
int
hs_net_server_accept(struct hs_net_server* server, int index);

// 读取数据到缓冲, 返回对应的agent
int
hs_net_server_recv(struct hs_net_server* server, int index, struct hs_net_agent** agent);

// 每循环调用的函数，处理连接、读写和错误事件
int
hs_net_server_run(struct hs_net_server* server);

// 设置处理数据函数
int
hs_net_server_data_handler(struct hs_net_server* server, data_handle handle_func);

// 设置连接成功处理函数
int
hs_net_server_conn_handler(struct hs_net_server* server, conn_handle handle_func);

// 设置断开连接处理函数
int
hs_net_server_disconn_handler(struct hs_net_server* server, disconn_handle handle_func);

// 错误处理函数
int
hs_net_server_err_handle(struct hs_net_server* server, err_handle handle_func);

#endif
