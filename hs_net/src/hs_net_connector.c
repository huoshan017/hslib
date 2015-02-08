#include "hs_net_connector.h"

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "hs_recvbuff.h"
#include "hs_sendbuff.h"
#include "hs_net_util.h"

// 创建连接者
struct hs_net_connector*
hs_net_connector_create(int recv_length, int send_length, bool is_reconnect)
{
	if (recv_length==0 || send_length==0)
		return NULL;

	struct hs_net_connector* connector = (struct hs_net_connector*)malloc(sizeof(struct hs_net_connector));
	if (!hs_net_connector_init(connector, recv_length, send_length, is_reconnect))
		return NULL;

	return connector;
}

// 销毁连接者
void
hs_net_connector_destroy(struct hs_net_connector* connector)
{
	if (connector) {
		hs_net_connector_uninit(connector);
		free(connector);
	}
}

// 初始化连接者
bool
hs_net_connector_init(struct hs_net_connector* connector, int recv_length, int send_length, bool reconnect)
{
	if (!connector || recv_length==0 || send_length==0)
		return false;
	
	memset(connector, 0, sizeof(*connector));
	connector->recv_buff = hs_recvbuff_create(recv_length);
	connector->send_buff = hs_sendbuff_create(send_length);
	connector->is_reconnect = reconnect;
	connector->reconnect_start_time = 0;

	return true;
}

// 反初始化连接者
void
hs_net_connector_uninit(struct hs_net_connector* connector)
{
	if (connector) {
		if (connector->recv_buff) {
			hs_recvbuff_destroy(connector->recv_buff);
			connector->recv_buff = NULL;
		}
		if (connector->send_buff) {
			hs_sendbuff_destroy(connector->send_buff);
			connector->send_buff = NULL;
		}
	}
}

// 连接
bool
hs_net_connector_connect(struct hs_net_connector* connector, const char* ip, short port)
{
	if (!connector)
		return false;

	int s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == -1) {
		printf("hs_net_connector_init: 建立connector套接字失败\n", s);
		return false;
	}

	connector->socket = s;

	// 设置非阻塞套接字
	hs_net_set_nonblocking(connector->socket);

	struct sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip);
	addr.sin_port = htons(port);

	int ip_len = strlen(ip);
	ip_len = ip_len < sizeof(connector->ip)-1 ? ip_len : sizeof(connector->ip)-1;
	memcpy(connector->ip, ip, ip_len);
	connector->port = port;

	int ret = connect(connector->socket, (struct sockaddr*)&addr, sizeof(addr));
	if (ret == -1) {
		if (errno == EINPROGRESS) {
			connector->state = CONN_STATE_CONNECTING;
			printf("hs_net_connector_connect: 连接正在进行\n");
			return true;
		}

		if (!connector->is_reconnect) {
			printf("hs_net_connector_connect: 连接(%s:%d)失败, error(%d)\n", ip, port, errno);
			return false;
		} else {
			connector->state = CONN_STATE_RECONNECTING;
			connector->reconnect_start_time = time(NULL);
			printf("hs_net_connector_connect: 连接(%s:%d)失败，准备重连\n", ip, port);
			return true;
		}
	} else if (ret == 0) {
		connector->state = CONN_STATE_CONNECTED;
		printf("hs_net_connector_connect: 连接成功\n");
		return true;
	}

	return false;
}

// 重新连接
bool
hs_net_connector_reconnect(struct hs_net_connector* connector)
{
	if (!connector)
		return false;

	return hs_net_connector_connect(connector, connector->ip, connector->port);
}

// 断开连接
void
hs_net_connector_disconnect(struct hs_net_connector* connector)
{
	if (!connector)
		return;

	shutdown(connector->socket, SHUT_RDWR);
	close(connector->socket);
	int s = connector->socket;
	connector->socket = 0;
	connector->state = CONN_STATE_DISCONNECT;
	hs_recvbuff_clear(connector->recv_buff);
	hs_sendbuff_clear(connector->send_buff);

	if (connector->handle_disconn) {
		connector->handle_disconn((void*)(long)s);
	}
}

// 写数据
int
hs_net_connector_write(struct hs_net_connector* connector, const char* data, int len)
{
	if (!connector)
		return -1;

	bool ret = hs_sendbuff_write(connector->send_buff, data, len);
	if (!ret) {
		printf("hs_net_connector_write: 写入失败\n");
		return 0;
	}

	return 1;
}

// 运行，返回值为-1说明套接字出错，建议关闭
int
hs_net_connector_run(struct hs_net_connector* connector)
{
	if (!connector)
		return -1;

	if (connector->state != CONN_STATE_CONNECTING
		&& connector->state != CONN_STATE_CONNECTED
		&& connector->state != CONN_STATE_RECONNECTING)
		return 0;

	struct timeval tv;
	memset(&tv, 0, sizeof(tv));
	int error, ret;
	socklen_t len;

	fd_set rdfs;
	FD_ZERO(&rdfs);
	FD_SET(connector->socket, &rdfs);

	fd_set wtfs;
	FD_ZERO(&wtfs);
	FD_SET(connector->socket, &wtfs);

	// 正在连接状态
	if (connector->state == CONN_STATE_CONNECTING) {
		ret = select(connector->socket+1, &rdfs, &wtfs, NULL, &tv);
		if (ret < 0) {
			hs_net_connector_disconnect(connector);
			printf("hs_net_connector_run: select 失败\n");
			return -1;
		} else if (ret > 0) {
			if (FD_ISSET(connector->socket, &rdfs) || FD_ISSET(connector->socket, &wtfs)) {
				len = sizeof(error);
				int code = getsockopt(connector->socket, SOL_SOCKET, SO_ERROR, &error, &len);
				if (code < 0 || error) {
					if (error == ECONNREFUSED) {
						connector->state = CONN_STATE_RECONNECTING;
						printf("hs_net_connector_run: 对方拒绝连接，准备重连\n");
						return 0;
					}
					hs_net_connector_disconnect(connector);
					printf("hs_net_connector_run: getsockpot failed (code:%d, error:%d)\n", code, error);
					return -1;
				} else {
					connector->state = CONN_STATE_CONNECTED;
					if (connector->handle_conn) {
						connector->handle_conn((void*)(long)connector);
					}
					
					printf("hs_net_connector_run: connect success (code: %d, error:%d)\n", code, error);
				}
			}
		}
	}
	// 已连接状态
	else if (connector->state == CONN_STATE_CONNECTED) {
		ret = select(connector->socket+1, &rdfs, &wtfs, NULL, &tv);
		if (ret < 0) {
			hs_net_connector_disconnect(connector);
			printf("hs_net_connector_run: select 失败\n");
			return -1;
		} else if (ret > 0) {
			if (FD_ISSET(connector->socket, &rdfs)) {
				int read_nbytes = 0;
				bool r = hs_recvbuff_writeto(connector->recv_buff, connector->socket, &read_nbytes);
				if (!r) {
					printf("hs_net_connector_run: 接收缓冲没有足够的空间\n");
				} else {
					if (read_nbytes < 0) {
						if (errno == EAGAIN) {
							printf("hs_net_connector_run: EAGAIN\n");
						} else {
							hs_net_connector_disconnect(connector);
							printf("hs_net_connector_run: receive data error\n");
							return -1;
						}
					} else if (read_nbytes == 0) {
						// 对方断开连接，设置计时器重连
						hs_net_connector_disconnect(connector);
						connector->state = CONN_STATE_RECONNECTING;
						connector->reconnect_start_time = time(NULL);
						printf("hs_net_connector_run: 对方断开连接，准备重连\n");
						return 0;
					}

					// 处理接收的数据
					int r = hs_recvbuff_data_handle(connector->recv_buff, connector->handle_data, (void*)(long)connector);
					if (r < 0) {
						printf("hs_net_connector_run: connector 处理数据错误\n");
						return -1;
					}
				}
			}

			if (FD_ISSET(connector->socket, &wtfs)) {
				int s = hs_sendbuff_send(connector->send_buff, connector->socket);
				if (s < 0) {
					hs_net_connector_disconnect(connector);
					printf("hs_net_connector_run: send data error\n");
					return -1;
				}
			}
		} else {
			printf("hs_net_connector_run: select return 0\n");
		}
	} else if (connector->state == CONN_STATE_RECONNECTING) {
		time_t now = time(NULL);
		if (now - connector->reconnect_start_time >= HS_DEFAULT_CONNECTOR_RECONNECT_INTERVAL) {
			if (!hs_net_connector_reconnect(connector)) {
				printf("hs_net_connector_run: reconnect failed\n");
			}
			connector->reconnect_start_time = now;
		}
	}

	return 1;
}

// 数据处理函数
int
hs_net_connector_data_handle(struct hs_net_connector* connector, data_handle handle_func)
{
	if (!connector)
		return -1;

	connector->handle_data = handle_func;

	return 1;
}

// 连接处理函数
int
hs_net_connector_conn_handle(struct hs_net_connector* connector, conn_handle handle_func)
{
	if (!connector)
		return -1;

	connector->handle_conn = handle_func;

	return 1;
}

// 断开处理函数
int
hs_net_connector_disconn_handle(struct hs_net_connector* connector, disconn_handle handle_func)
{
	if (!connector)
		return -1;

	connector->handle_disconn = handle_func;

	return 1;
}
