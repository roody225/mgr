#ifndef _OFFLOAD_H_
#define _OFFLOAD_H_

#include <stdint.h>
#include <stdlib.h>

#define nl_mgrp(x) (1 << (x-1))

#define MULTICAST_GROUPS \
    nl_mgrp(RTNLGRP_LINK) \
    | nl_mgrp(RTNLGRP_IPV4_ROUTE) \
    | nl_mgrp(RTNLGRP_NEIGH)

int netlink_connect();
void netlink_disconnect();

void *listen_loop(void *arg);
void wait_for_listen_loop();
void listen_loop_stop();

void send_netlink_message(uint16_t type, void *data, size_t data_size, uint16_t flags);

#endif /* _OFFLOAD_H_ */
