#include "hs_net_agent.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/epoll.h>
#include "hs_sendbuff.h"
#include "hs_recvbuff.h"

struct hs_net_agent*
hs_net_new_agent(int sock, int recv_length, int send_length) {
	struct hs_net_agent* agent = (struct hs_net_agent*)malloc(sizeof(struct hs_net_agent));
	agent->socket = sock;
	agent->recv_buff = hs_recvbuff_create(recv_length);
	agent->send_buff = hs_sendbuff_create(send_length);
	return agent;
}

// 代理写入数据到缓冲
int
hs_net_agent_write(struct hs_net_agent* agent, const char* buf, int len)
{
	if (!agent)
		return -1;

	bool res = hs_sendbuff_write(agent->send_buff, buf, len);
	if (!res) {
		printf("hs_server_write: 无法写到发送缓冲\n");
		return -1;
	}

	return 1;
}

// 关闭代理
int
hs_net_agent_close(struct hs_net_server* server, struct hs_net_agent* agent)
{
	if (!server || !agent)
		return -1;

	int s = agent->socket;
	close(agent->socket);
	epoll_ctl(server->epoll_fd, EPOLL_CTL_DEL, agent->socket, server->events);
	hs_recvbuff_destroy(agent->recv_buff);
	hs_sendbuff_destroy(agent->send_buff);
	hs_hashmap_delete(server->agents, agent->socket, NULL);

	// 断线回调处理
	if (server->handle_disconn) {
		void* param = (void*)(long)s;
		server->handle_disconn(param);
	}
	return 1;
}
