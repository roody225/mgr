#ifndef _NETLINK_HANDLERS_H_
#define _NETLINK_HANDLERS_H_

#include <stdint.h>
#include <linux/if_ether.h>
#include <linux/neighbour.h>

#define IPV6_ADDR_LEN 16

struct link_message {
    uint32_t id;
    uint32_t vrf;
    uint8_t is_vrf_slave:1;
    uint8_t reserved:7;
    uint8_t mac_addr[ETH_ALEN];
};

struct route_message {
    uint32_t addr;
    uint32_t vrf;
    uint32_t dev;
    uint32_t gw;
    uint8_t is_ip4:1;
    uint8_t prefix_len:7;
    uint8_t addr6[IPV6_ADDR_LEN];
    uint8_t gw6[IPV6_ADDR_LEN];
};

struct neigh_message {
    uint32_t gw;
    uint16_t state;
    uint8_t addr[ETH_ALEN];
    uint8_t gw6[IPV6_ADDR_LEN];
    uint8_t is_ip4:1;
    uint8_t reserved:7;
};

struct netlink_info {
    int type;
    union {
        struct link_message lm;
        struct route_message rm;
        struct neigh_message nm;
    };
};

static const uint16_t ACTIVE_NEIGH_STATES_MASK =
    NUD_REACHABLE
    | NUD_STALE
    | NUD_DELAY
    | NUD_PROBE
    | NUD_PERMANENT;

int handle_newlink(const struct nlmsghdr *h, struct link_message *lm);
int handle_dellink(const struct nlmsghdr *h, struct link_message *lm);

int handle_newroute(const struct nlmsghdr *h, struct route_message *rm);
int handle_delroute(const struct nlmsghdr *h, struct route_message *rm);

int handle_newneigh(const struct nlmsghdr *h, struct neigh_message *nm);
int handle_delneigh(const struct nlmsghdr *h, struct neigh_message *nm);

#endif /* _NETLINK_HANDLERS_H_ */
