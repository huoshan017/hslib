#include "hs_net_channel.h"

struct hs_net_channel*
hs_net_channel_create(int recv_length, int send_length)
{
	struct hs_net_channel* channel = (struct hs_net_channel*)malloc(sizeof(struct hs_net_channel));
	char conn_ip[16];
	char listen_ip[16];
	short conn_port;
	short listen_port
	channel->
}

void
hs_net_channel_destroy(struct hs_net_channel* channel)
{
}

bool
hs_net_channel_init(struct hs_net_channel* channel, int recv_length, int send_length)
{
}

void
hs_net_channel_uninit(struct hs_net_channel* channel)
{
}

bool
hs_net_channel_start(struct hs_net_channel* channel, const char* conn_ip, short conn_port, const char* listen_ip, short listen_port)
{
}

bool
hs_net_channel_restart(struct hs_net_channel* channel)
{
}

void
hs_net_channel_end(struct hs_net_channel* channel)
{
}

int
hs_net_channel_write(struct hs_net_channel* channel, const char* buf, int len)
{
}

int
hs_net_channel_run(struct hs_net_channel* channel)
{
}

int
hs_net_channel_data_handle(struct hs_net_channel* channel, data_handle handle_func)
{
}

int
hs_net_channel_conn_handle(struct hs_net_channel* channel, conn_handle handle_func)
{
}

int
hs_net_channel_disconn_handle(struct hs_net_channel* channel, disconn_handle handle_func)
{
}

