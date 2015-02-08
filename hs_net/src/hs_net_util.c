#include "hs_net_util.h"

#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

void
hs_net_set_nonblocking(int socket)
{
	int opts;
	opts = fcntl(socket, F_GETFL);
	if (opts < 0) {
		perror("fcntl(sock, F_GETFL)");
		exit(1);
	}
	opts = opts | O_NONBLOCK;
	if (fcntl(socket, F_SETFL, opts) < 0) {
		perror("fcntl(sock, F_SETFL, opts)");
		exit(1);
	}
}
