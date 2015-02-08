#include "hs_net_server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>

#include "hs_net_agent.h"
#include "hs_hashmap.h"
#include "hs_recvbuff.h"
#include "hs_sendbuff.h"
#include "hs_net_util.h"

static const uint32_t DEFAULT_RECV_BUFF_LENGTH = 8192 * 16;
static const uint32_t DEFAULT_SEND_BUFF_LENGTH = 1024 * 16;


// 新建服务器
struct hs_net_server*
hs_net_server_create(short port, uint32_t agent_size)
{
	int listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	printf("new socket %d\n", listen_socket);
	hs_net_set_nonblocking(listen_socket);

	struct sockaddr_in servaddr;
	memset((void*)&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	bzero(&servaddr.sin_zero, sizeof(servaddr.sin_zero));

	// bind
	int res = bind(listen_socket, (struct sockaddr*)&servaddr, sizeof(servaddr));
	if (res == -1) {
		close(listen_socket);
		perror("bind");
		return NULL;
	}

	printf("bind socket %d\n", listen_socket);

	// listen
	res = listen(listen_socket, 10);
	if (res == -1) {
		close(listen_socket);
		perror("listen");
		return NULL;
	}

	printf("listen socket %d\n", listen_socket);

	// register epoll event
	int epfd = epoll_create(agent_size);
	printf("epoll created %d\n", epfd);
	static struct epoll_event ev;
	ev.data.fd = listen_socket;
	ev.events = EPOLLIN | EPOLLET;
	res = epoll_ctl(epfd, EPOLL_CTL_ADD, listen_socket, &ev);
	if (res == -1) {
		close(listen_socket);
		perror("epoll_ctl");
		return NULL;
	}

	// malloc server memory
	struct hs_net_server* svr = (struct hs_net_server*)malloc(sizeof(struct hs_net_server));
	svr->listen_socket = listen_socket;
	svr->epoll_fd = epfd;
	svr->agents = hs_hashmap_create(agent_size);
	svr->events = (struct epoll_event*)malloc(sizeof(struct epoll_event)*agent_size);
	svr->events_maxnum = agent_size;
	svr->handle_data = NULL;
	svr->handle_conn = NULL;
	svr->handle_disconn = NULL;
	svr->handle_err = NULL;

	printf("hs_server created\n");

	return svr;
}

// 销毁服务器
void
hs_net_server_destroy(struct hs_net_server* server)
{
	hs_net_server_close(server);
	if (server) {
		free(server);
	}
}

// 关闭服务器
void
hs_net_server_close(struct hs_net_server* server)
{
	if (!server)
		return;

	// 遍历并关闭所有socket
	struct hs_hashmap_iter iter = hs_hashmap_iter_init(server->agents);
	void* data = NULL;
	while (1) {
		bool res = hs_hashmap_iter_data(&iter, &data);
		struct hs_net_agent* agent = (struct hs_net_agent*)data;
		if (agent) {
			hs_net_agent_close(server, agent);
		}
		if (!hs_hashmap_iter_next(&iter))
			break;
	}
	hs_hashmap_destroy(server->agents);
	server->agents = NULL;
	close(server->listen_socket);
	close(server->epoll_fd);

	epoll_ctl(server->epoll_fd, EPOLL_CTL_DEL, server->listen_socket, NULL);
}

// 等待事件，返回能读取的agent个数
int
hs_net_server_wait(struct hs_net_server* server)
{
	if (!server)
		return -1;

	int nfds = epoll_wait(server->epoll_fd, server->events, server->events_maxnum, 0);
	//printf("epoll_wait return %d\n", nfds);
	return nfds;
}

// 连接事件
static int
_accept_event(struct hs_net_server* server, int listen_socket) {
	// 加入到epoll等待队列
	struct sockaddr_in clientaddr;
	socklen_t addrlen = 0;
	while (1) {
		int new_fd = accept(listen_socket, (struct sockaddr*)&clientaddr, &addrlen);
		if (new_fd <= 0) {
			if (errno == EAGAIN) {
				// 没有连接需要接收了
				break;
			} else if (errno == EINTR) {
				// 可能被中断信号打断，经过验证对非阻塞socket并未接收到此错误，应该可以省掉该步判断
				break;
			} else {
				// 其他情况可以认为该描述字出现错误，应该关闭后重新监听
				return -1;
			}
		}
		
		// 设为非阻塞模式
		hs_net_set_nonblocking(new_fd);
		struct epoll_event ev;
		ev.data.fd = new_fd;
		ev.events = EPOLLIN | EPOLLOUT | EPOLLET | EPOLLHUP | EPOLLRDHUP | EPOLLERR;
		epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, new_fd, &ev);

		struct hs_net_agent* agent = hs_net_new_agent(new_fd, DEFAULT_RECV_BUFF_LENGTH, DEFAULT_SEND_BUFF_LENGTH);
		if (!hs_hashmap_insert(server->agents, new_fd, agent)) {
			printf("插入新代理(%d)失败\n", new_fd);
			return 0;
		}
		
		if (server->handle_conn) {
			server->handle_conn((void*)agent);
		}

		printf("%s(%d): 新的连接，套接字句柄(%d)\n", __FILE__, __LINE__, new_fd);
	}
	return 1;
}

// 接收数据事件
static int
_recv_event(struct hs_net_server* server, struct hs_net_agent* agent) {
	int n = 0;
	while (1) {
		int nbytes = 0;
		bool res = hs_recvbuff_writeto(agent->recv_buff, agent->socket, &nbytes);
		if (!res) {
			// 不能写入，处理已接收的数据
			int n = hs_recvbuff_data_handle(agent->recv_buff, server->handle_data, (void*)agent);
			if (n < 0) {
				// 关闭套接字
				printf("处理套接字(%d)数据出错\n", agent->socket);
				return -1;
			} else {
				if (n == 0)
					break;

				printf("处理了套接字(%d)接收的数据(%d)个字节\n", agent->socket, n);
			}
		} else {
			if (nbytes < 0) {
				// 读取socket出错
				if (errno == EAGAIN) {
					// 数据已读完
					printf("套接字(%d)数据已读完, errno(%d)\n", agent->socket, errno);
					break;
				} else if (errno == EINTR) {
					// 可能被内部中断信号打断，经过验证对非阻塞socket并未收到此错误，应该可以省掉该步骤
					printf("套接字(%d)内部中断信号，errno(%d)\n", agent->socket, errno);
					return -1;
				} else {
					// 客户端主动关闭
					printf("套接字(%d)被对方关闭: errno(%d)\n", agent->socket, errno);
					return -1;
				}
			} else if (nbytes > 0) {
				// 读到数据
				// 综合两种情况，在读到字节数大于0时必须继续，不管读到是否等于接收缓冲区大小，
				// 也不管错误代码是否为EAGAIN，否则要么导致关闭事件丢失，要么导致后续数据的丢失
				int n = hs_recvbuff_data_handle(agent->recv_buff, server->handle_data, (void*)agent);
				if (n < 0) {
					// 关闭套接字
					printf("处理套接字(%d)出错\n", agent->socket);
					return -1;
				}
				n += nbytes;
				continue;
			} else {
				// 读不到时跳出
				printf("套接字(%d)已读不到数据\n", agent->socket);
				break;
			}
		}
	}
	return n;
}

// 发送数据事件
static int
_send_event(struct hs_net_server* server, struct hs_net_agent* agent) {
	int n = hs_sendbuff_send(agent->send_buff, agent->socket);
	if (n < 0)
		return -1;
	return n;
}

// 处理事件
int
hs_net_server_process_event(struct hs_net_server* server, int index)
{
	if (!server)
		return HS_NET_NOT_FOUND_SERVER;

	struct epoll_event* ee = &server->events[index];
	// 处理连接事件
	if (ee->data.fd == server->listen_socket) {
		if (ee->events & EPOLLIN) {
			int res = _accept_event(server, server->listen_socket);
			if (res < 0)
				return HS_NET_LISTEN_ERROR;
		} else if (ee->events & EPOLLERR || ee->events & EPOLLHUP) {
			return HS_NET_LISTEN_ERROR;
		}
	} else {
		struct hs_net_agent* agent = NULL;
		bool res = hs_hashmap_find(server->agents, ee->data.fd, (void**)&agent);
		if (!res) {
			printf("套接字(%d)找不到对应的agent\n", ee->data.fd);
			return HS_NET_NOT_FOUND_AGENT;
		}

		// 有数据到来
		if (ee->events & EPOLLIN) {
			int n = _recv_event(server, agent);
			if (n < 0) {
				// 删除代理
				hs_net_agent_close(server, agent);
				return HS_NET_RECV_ERROR;
			}
		}
		// 可以写数据
		if (ee->events & EPOLLOUT) {
			int n = _send_event(server, agent);
			if (n < 0) {
				hs_net_agent_close(server, agent);
				return HS_NET_SEND_ERROR;
			}
		}
		// 有异常发生，断开连接
		if (ee->events & EPOLLERR || ee->events & EPOLLHUP) {
			hs_net_agent_close(server, agent);
			printf("hs_server_process_event: 有异常发生，断开客户端连接\n");
			return HS_NET_CLIENT_DISCONNECT;
		}
		// 客户端主动断开连接
		if (ee->events & EPOLLRDHUP) {
			hs_net_agent_close(server, agent);
			printf("hs_server_process_event: 客户端主动断开连接\n");
			return HS_NET_CLIENT_DISCONNECT;
		}
	}

	return HS_NET_OK;
}

// 处理新连接，返回-1表示套接字已出错，需要重新创建
int
hs_net_server_accept(struct hs_net_server* server, int index)
{
	if (!server)
		return -1;

	struct epoll_event* ee = &server->events[index];
	if (ee->data.fd != server->listen_socket)
		return 0;

	if (ee->events & EPOLLIN) {
		int res = _accept_event(server, server->listen_socket);
		if (res < 0)
			return -1;
	} else if (ee->events & EPOLLERR || ee->events & EPOLLHUP) {
		// 有异常发生
		return -1;
	}
	return 1;
}

// 读取数据到缓冲, 返回对应的agent
int
hs_net_server_recv(struct hs_net_server* server, int index, struct hs_net_agent** agent)
{
	if (!server)
		return -1;

	struct epoll_event* ee = &server->events[index];
	if (ee->data.fd == server->listen_socket)
		return 0;

	// 有数据到来
	if (ee->events & EPOLLIN) {
		bool res = hs_hashmap_find(server->agents, ee->data.fd, (void**)agent);
		if (!res) {
			printf("套接字(%d)找不到对应的agent\n", ee->data.fd);
			return -1;
		}
		int n = _recv_event(server, *agent);
		if (n < 0)
			return -1;
	} else if (ee->events & EPOLLOUT) {
		// 可以写数据
	} else if (ee->events & EPOLLERR || ee->events & EPOLLHUP) {
		// 有异常发生
		return -1;
	}

	return 1;
}

// 处理数据
int
hs_net_server_data_handle(struct hs_net_server* server, data_handle handle_func)
{
	if (!server)
		return -1;

	server->handle_data = handle_func;

	return 1;
}

// 设置连接成功处理函数
int
hs_net_server_conn_handler(struct hs_net_server* server, conn_handle handle_func)
{
	if (!server)
		return -1;

	server->handle_conn = handle_func;

	return 1;
}

// 设置断开连接处理函数
int
hs_net_server_disconn_handler(struct hs_net_server* server, disconn_handle handle_func)
{
	if (!server)
		return -1;

	server->handle_disconn = handle_func;

	return 1;
}

// 错误处理函数
int
hs_server_err_handle(struct hs_net_server* server, err_handle handle_func)
{
	if (!server)
		return -1;

	server->handle_err = handle_func;

	return 1;
}

// 每循环调用的函数
int
hs_net_server_run(struct hs_net_server* server)
{
	if (!server)
		return -1;

	int nfd = hs_net_server_wait(server);
	if (nfd < 0) {
		hs_net_server_close(server);
		printf("waiting agent connet or data receive failed\n");
		return -1;
	}

	int i = 0;
	for (; i<nfd; ++i) {
		int res = hs_net_server_process_event(server, i);
		if (res == HS_NET_NOT_FOUND_SERVER || res == HS_NET_LISTEN_ERROR) {
			hs_net_server_close(server);
			return -1;
		}
	}

	return 1;
}
