#ifndef __HS_NET_AGENT_H__
#define __HS_NET_AGENT_H__

#include "hs_netdef.h"

// 新代理
struct hs_net_agent*
hs_net_new_agent(int sock, int recv_length, int send_length);

// 写入数据到代理
int
hs_net_agent_write(struct hs_net_agent* agent, const char* buf, int len);

// 关闭代理
int
hs_net_agent_close(struct hs_net_server* server, struct hs_net_agent* agent);

#endif
