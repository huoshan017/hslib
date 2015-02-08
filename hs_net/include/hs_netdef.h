#ifndef __HS_BASE_H__
#define __HS_BASE_H__

static const int HS_DEFAULT_CONNECTOR_RECV_BUFF_LENGTH = 1024 * 32;
static const int HS_DEFAULT_CONNECTOR_SEND_BUFF_LENGTH = 1024 * 32;
static const int HS_DEFAULT_ACCEPTOR_AGENT_RECV_BUFF_LENGTH = 1024 * 16;
static const int HS_DEFAULT_ACCEPTOR_AGENT_SEND_BUFF_LENGTH = 1024 * 16;

static const int HS_DEFAULT_CONNECTOR_RECONNECT_INTERVAL = 5;

// 处理网络事件错误码
static const int HS_NET_OK = 1;
static const int HS_NET_NOT_FOUND_SERVER 	= -1;		// 找不到服务器
static const int HS_NET_NOT_FOUND_AGENT 	= -2;		// 找不到代理
static const int HS_NET_LISTEN_ERROR 		= -1000;	// 监听出错
static const int HS_NET_RECV_ERROR 			= -1001;	// 接收数据出错
static const int HS_NET_SEND_ERROR 			= -1002;	// 发送数据出错
static const int HS_NET_CLIENT_DISCONNECT 	= -2000;	// 客户端断开

// 数据处理函数指针
typedef int (*data_handle)(char* buf, int len, void* param);

// 连接建立
typedef int (*conn_handle)(void* param);

// 连接断开
typedef int (*disconn_handle)(void* param);

// 连接出错
typedef int (*err_handle)(void* param);

struct hs_recvbuff;
struct hs_sendbuff;
struct epoll_event;
struct hs_hashmap;
struct hs_array;

struct hs_net_agent {
	int 				socket;
	struct hs_recvbuff* recv_buff;
	struct hs_sendbuff* send_buff;
};

struct hs_net_server {
	int 				listen_socket;
	int 				epoll_fd;
	struct 				hs_hashmap* agents;
	struct epoll_event* events;
	int 				events_maxnum;
	data_handle 		handle_data;
	conn_handle 		handle_conn;
	disconn_handle 		handle_disconn;
	err_handle 			handle_err;
};

enum ConnectorState {
	CONN_STATE_NOT_CONNECTED 	= -1,
	CONN_STATE_CONNECTING 		= 0,
	CONN_STATE_CONNECTED 		= 1,
	CONN_STATE_DISCONNECT 		= 2,
	CONN_STATE_RECONNECTING 	= 3,
};

#include <stdbool.h>
#include <time.h>

struct hs_net_connector {
	char 				ip[16];
	short 				port;
	int 				socket;
	enum ConnectorState	state;
	struct hs_recvbuff* recv_buff;
	struct hs_sendbuff* send_buff;
	bool				is_reconnect;
	time_t				reconnect_start_time;
	data_handle			handle_data;
	conn_handle			handle_conn;
	disconn_handle		handle_disconn;
};

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <netinet/in.h>


struct hs_net_acceptor {
	char 				ip[16];
	short 				port;
	int 				socket;
	fd_set 				rset;
	fd_set				wset;
	struct timeval 		tv;
	struct sockaddr_in 	addr;
	socklen_t 			addrlen;
	int 				agent_recvbuff_length;
	int					agent_sendbuff_length;
	struct hs_array* 	agents;
	int					max_agents_num;
	int					curr_agents_num;
	data_handle 		handle_data;
	conn_handle 		handle_conn;
	disconn_handle 		handle_disconn;
};

#endif
