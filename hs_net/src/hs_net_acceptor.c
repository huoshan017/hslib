#include "hs_net_acceptor.h"

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "hs_array.h"
#include "hs_recvbuff.h"
#include "hs_sendbuff.h"
#include "hs_net_agent.h"
#include "hs_net_util.h"

struct hs_net_acceptor*
hs_net_acceptor_create(int agent_max_num, int agent_recv_length, int agent_send_length, short port)
{
	if (agent_recv_length==0 || agent_send_length==0)
		return NULL;

	struct hs_net_acceptor* acceptor = (struct hs_net_acceptor*)malloc(sizeof(struct hs_net_acceptor));
	if (!hs_net_acceptor_init(acceptor, agent_max_num, agent_recv_length, agent_send_length, port))
		return NULL;

	return acceptor;
}

void
hs_net_acceptor_destroy(struct hs_net_acceptor* acceptor)
{
	if (acceptor) {
		hs_net_acceptor_uninit(acceptor);
		free(acceptor);
	}
}

bool
hs_net_acceptor_init(struct hs_net_acceptor* acceptor, int agent_max_num, int agent_recv_length, int agent_send_length, short port)
{
	if (!acceptor || agent_recv_length==0 || agent_send_length==0)
		return false;

	acceptor->socket = socket(AF_INET, SOCK_STREAM, 0);
	if (acceptor->socket < 0) {
		printf("hs_net_acceptor_init: 创建套接字失败\n");
		return false;
	}

	hs_net_set_nonblocking(acceptor->socket);

	acceptor->agent_recvbuff_length = agent_recv_length;
	acceptor->agent_sendbuff_length = agent_send_length;
	acceptor->agents = hs_array_create(agent_max_num);
	acceptor->max_agents_num = agent_max_num;
	acceptor->curr_agents_num = 0;
	acceptor->handle_conn = NULL;
	acceptor->handle_data = NULL;
	acceptor->handle_disconn = NULL;

	if (hs_net_acceptor_listen(acceptor, port) < 0) {
		return false;
	}

	return true;
}

static void
_agent_destroy(struct hs_net_acceptor* acceptor, struct hs_net_agent* agent) {
	if (!agent)
		return;

	int s = agent->socket;
	close(agent->socket);
	hs_recvbuff_destroy(agent->recv_buff);
	hs_sendbuff_destroy(agent->send_buff);
	hs_array_delete_first(acceptor->agents, (void*)(long)agent->socket);
	free(agent);

	// 断线回调处理
	if (acceptor->handle_disconn) {
		void* param = (void*)(long)s;
		acceptor->handle_disconn(param);
	}

	printf("_agent_destroy: %d\n", s);
}

void
hs_net_acceptor_uninit(struct hs_net_acceptor* acceptor)
{
	if (acceptor) {
		close(acceptor->socket);
		if (acceptor->agents) {
			// 遍历并关闭所有socket
			size_t elem_num = hs_array_elem_num(acceptor->agents);
			size_t i = 0;
			for (; i<elem_num; ++i) {
				struct hs_net_agent* agent = (struct hs_net_agent*)hs_array_get_elem(acceptor->agents, i);
				if (agent) {
					_agent_destroy(acceptor, agent);
				}
			}
			hs_array_destroy(acceptor->agents);
			acceptor->agents = NULL;
		}
	}
}

int
hs_net_acceptor_listen(struct hs_net_acceptor* acceptor, short port)
{
	if (!acceptor)
		return -1;

	struct sockaddr_in servaddr;
	memset((void*)&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	bzero(&servaddr.sin_zero, sizeof(servaddr.sin_zero));

	// bind
	int res = bind(acceptor->socket, (struct sockaddr*)&servaddr, sizeof(servaddr));
	if (res == -1) {
		perror("bind");
		return -1;
	}

	printf("bind socket %d\n", acceptor->socket);

	// listen
	res = listen(acceptor->socket, 0);
	if (res == -1) {
		perror("listen");
		return -1;
	}

	printf("listen socket %d\n", acceptor->socket);

	return 1;
}

int
_listen_socket_run(struct hs_net_acceptor* acceptor) {
	FD_ZERO(&acceptor->rset);
	memset(&acceptor->tv, 0, sizeof(acceptor->tv));
	FD_SET(acceptor->socket, &acceptor->rset);
	int ret = select(acceptor->socket+1, &acceptor->rset, NULL, NULL, &acceptor->tv);
	if (ret < 0) {
		printf("_listen_socket_run: select failed\n");
		return -1;
	} else if (ret == 0) {
		return 0;
	}

	if (FD_ISSET(acceptor->socket, &acceptor->rset)) {
		acceptor->addrlen = sizeof(acceptor->addr);
		int new_fd = accept(acceptor->socket, (struct sockaddr*)&acceptor->addr, &acceptor->addrlen);
		if (new_fd <= 0) {
			printf("_listen_socket_run: accept error\n");
			return 0;
		}
		size_t cap = hs_array_capacity(acceptor->agents);
		size_t elem_num = hs_array_elem_num(acceptor->agents);
		if (elem_num >= cap) {
			shutdown(new_fd, SHUT_RDWR);
			close(new_fd);
			printf("_listen_socket_run: 连接数(%d)已打最大\n", elem_num);
			return 0;
		}

		struct hs_net_agent*
			agent = hs_net_new_agent(new_fd, HS_DEFAULT_ACCEPTOR_AGENT_RECV_BUFF_LENGTH, HS_DEFAULT_ACCEPTOR_AGENT_SEND_BUFF_LENGTH);
		if (!hs_array_push_elem(acceptor->agents, (void*)agent)) {
			_agent_destroy(acceptor, agent);
			return 0;
		}

		if (acceptor->handle_conn) {
			acceptor->handle_conn((void*)(long)agent);
		}

		printf("_listen_socket_run: 新的连接(%d)\n", agent->socket);

		return 1;
	}

	return 0;
}

int
_agent_socket_run(struct hs_net_acceptor* acceptor, struct hs_net_agent* agent) {
	FD_ZERO(&acceptor->rset);
	FD_ZERO(&acceptor->wset);
	memset(&acceptor->tv, 0, sizeof(acceptor->tv));
	FD_SET(agent->socket, &acceptor->rset);
	FD_SET(agent->socket, &acceptor->wset);
	int ret = select(agent->socket+1, &acceptor->rset, &acceptor->wset, NULL, &acceptor->tv);
	if (ret < 0) {
		printf("_agent_socket_run: select failed\n");
		return -1;
	} else if (ret == 0) {
		return 0;
	}

	if (FD_ISSET(agent->socket, &acceptor->rset)) {
		// 可读
		bool is_write = hs_recvbuff_writeto(agent->recv_buff, agent->socket, &ret);
		if (!is_write) {
			int n = hs_recvbuff_data_handle(agent->recv_buff, acceptor->handle_data, (void*)agent);
			if (n < 0) {
				printf("_agent_socket_run: 处理套接字(%d)数据出错\n", agent->socket);
				return -1;
			} else {
				printf("_agent_socket_run: 处理了套接字(%d)接收的数据(%d)个字节\n", agent->socket, n);
			}
		} else {
			if (ret <= 0) {
				printf("_agent_socket_run: 套接字(%d)接收数据错误\n", agent->socket);
				return -1;
			} else if (ret > 0) {
				int n = hs_recvbuff_data_handle(agent->recv_buff, acceptor->handle_data, (void*)agent);
				if (n < 0) {
					printf("_agent_socket_run: 处理套接字(%d)出错\n", agent->socket);
					return -1;
				}
			}
		}
	}
	//if (FD_ISSET(agent->socket, &acceptor->wset)) {
		// 可写
		int n = hs_sendbuff_send(agent->send_buff, agent->socket);
		if (n < 0) {
			printf("_agent_socekt_run: 套接字(%d)发送缓冲发送数据出错\n", agent->socket);
			return -1;
		}
		if (n > 0) {
			printf("_agent_socket_run: 套接字(%d)发送了(%d)个字节的数据\n", agent->socket, n);
		}
	//} 
	return 1;
}

int
hs_net_acceptor_run(struct hs_net_acceptor* acceptor)
{
	if (!acceptor)
		return -1;

	if (!acceptor->agents)
		return -1;

	
	if (_listen_socket_run(acceptor) < 0) {
		return -1;
	}
	

	struct hs_net_agent* agent = NULL;
	size_t i = 0;
	for ( ; ; ) {
		if (i >= hs_array_elem_num(acceptor->agents)) 
			break;

		agent = (struct hs_net_agent*)hs_array_get_elem(acceptor->agents, i);
		if (!agent) {
			i += 1;
			continue;
		}

		if (_agent_socket_run(acceptor, agent) < 0) {
			_agent_destroy(acceptor, agent);
			hs_array_delete_byindex(acceptor->agents, i);
		} else {
			i += 1;
		}
	}

	return 1;
}

int
hs_net_acceptor_agent_data_handle(struct hs_net_acceptor* acceptor, data_handle handle_func)
{
	if (!acceptor)
		return -1;

	acceptor->handle_data = handle_func;

	return 1;
}

int
hs_net_acceptor_agent_conn_handle(struct hs_net_acceptor* acceptor, conn_handle handle_func)
{
	if (!acceptor)
		return -1;

	acceptor->handle_conn = handle_func;

	return 1;
}

int
hs_net_acceptor_agent_disconn_handle(struct hs_net_acceptor* acceptor, disconn_handle handle_func)
{
	if (!acceptor)
		return -1;

	acceptor->handle_disconn = handle_func;

	return 1;
}
