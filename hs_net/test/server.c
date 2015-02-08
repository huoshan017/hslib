#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "hs_net_server.h"
#include "hs_hashmap.h"

static const short SERVER_PORT = 10111;
static const int max_agent_size = 4;

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
	struct hs_net_server* hs = hs_net_server_create(SERVER_PORT, max_agent_size);
	if (!hs) {
		printf("create server failed\n");
		return -1;
	}

	hs_net_server_data_handle(hs, my_data_proc);

	printf("server start\n");
	
	int res = 0;
	while (1) {
		res = hs_net_server_run(hs);
		if (res < 0) {
			printf("hs_server_run: server循环失败\n");
			break;
		}
		usleep(1000);
	}

	hs_net_server_destroy(hs);

	return res;
}
