#ifndef __HS_NET_ACCEPTOR_H__
#define __HS_NET_ACCEPTOR_H__

#include <stdlib.h>
#include <stdbool.h>
#include "hs_netdef.h"

struct hs_net_acceptor*
hs_net_acceptor_create(int agent_max_num, int agent_recv_length, int agent_send_length, short port);

void
hs_net_acceptor_destroy(struct hs_net_acceptor* acceptor);

bool
hs_net_acceptor_init(struct hs_net_acceptor* acceptor, int agent_max_num, int agent_recv_length, int agent_send_length, short port);

void
hs_net_acceptor_uninit(struct hs_net_acceptor* acceptor);

int
hs_net_acceptor_listen(struct hs_net_acceptor* acceptor, short port);

int
hs_net_acceptor_run(struct hs_net_acceptor* accetpor);

int
hs_net_acceptor_agent_data_handle(struct hs_net_acceptor* acceptor, data_handle handle_func);

int
hs_net_acceptor_agent_conn_handle(struct hs_net_acceptor* acceptor, conn_handle handle_func);

int
hs_net_acceptor_agent_disconn_handle(struct hs_net_acceptor* acceptor, disconn_handle handle_func);

#endif
