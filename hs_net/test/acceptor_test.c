#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include "hs_netdef.h"
#include "hs_net_acceptor.h"
#include "hs_net_agent.h"

static const char* SERVER_IP = "192.168.1.106";
static const short SERVER_PORT = 10111;

static int my_conn_proc(void* param) {
	struct hs_net_agent* agent = (struct hs_net_agent*)param;
	if (agent) {
		printf("agent %d connected\n", agent->socket);
	}
	return 1;
}

static int my_disconn_proc(void* param) {
	int s = (int)(long)param;
	printf("agent %d disconnected\n", s);
	return 1;
}

static int my_data_proc(char* buf, int len, void* param) {
	char temp[4096];
	if (sizeof(temp)-1 < len)
		len = sizeof(temp)-1;
	memcpy(temp, buf, len);
	printf("get the data (%s)\n", temp);

	struct hs_net_agent* agent = (struct hs_net_agent*)param;
	hs_net_agent_write(agent, temp, len);
	return len;
}

int main(char** argc, int argv) {
	struct hs_net_acceptor* ha = hs_net_acceptor_create(4, HS_DEFAULT_ACCEPTOR_AGENT_RECV_BUFF_LENGTH, HS_DEFAULT_ACCEPTOR_AGENT_SEND_BUFF_LENGTH, SERVER_PORT);
	if (!ha) {
		printf("create acceptor failed\n");
		return -1;
	}

	hs_net_acceptor_agent_data_handle(ha, my_data_proc);
	hs_net_acceptor_agent_conn_handle(ha, my_conn_proc);
	hs_net_acceptor_agent_disconn_handle(ha, my_disconn_proc);

	printf("acceptor start\n");
	
	int res = 0;
	while (1) {
		res = hs_net_acceptor_run(ha);
		if (res < 0) {
			printf("hs_net_acceptor_run: server循环失败\n");
			break;
		}
		usleep(1000);
	}

	hs_net_acceptor_destroy(ha);

	return res;
}
