#ifndef __HS_NET_CONNECTOR_H__
#define __HS_NET_CONNECTOR_H__

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "hs_netdef.h"

// 创建连接者
struct hs_net_connector*
hs_net_connector_create(int recv_length, int send_length, bool is_reconnect);

// 销毁连接者
void
hs_net_connector_destroy(struct hs_net_connector* connector);

// 初始化连接者
bool
hs_net_connector_init(struct hs_net_connector* connector, int recv_length, int send_length, bool is_reconnect);

// 反初始化连接者
void
hs_net_connector_uninit(struct hs_net_connector* connector);

// 连接
bool
hs_net_connector_connect(struct hs_net_connector* connector, const char* ip, short port);

// 重新连接
bool
hs_net_connector_reconnect(struct hs_net_connector* connector);

// 断开连接
void
hs_net_connector_disconnect(struct hs_net_connector* connector);

// 写数据
int
hs_net_connector_write(struct hs_net_connector* connector, const char* data, int length);

// 运行
int
hs_net_connector_run(struct hs_net_connector* connector);

// 数据处理函数
int
hs_net_connector_data_handle(struct hs_net_connector* connector, data_handle handle_func);

// 连接处理函数
int
hs_net_connector_conn_handle(struct hs_net_connector* connector, conn_handle handle_func);

// 断开处理函数
int
hs_net_connector_disconn_handle(struct hs_net_connector* connector, disconn_handle handle_func);

#endif
