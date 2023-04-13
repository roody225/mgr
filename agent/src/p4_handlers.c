#include <stdio.h>

#include <linux/rtnetlink.h>
#include <arpa/inet.h>

#include "p4_handlers.h"

int insert_newlink(struct link_message *lm)
{
    printf("INFO: new link %d table %d\n", lm->id, (lm->is_vrf_slave ? lm->vrf : RT_TABLE_MAIN));
    return 0;
}
int insert_dellink(struct link_message *lm)
{
    printf("INFO: del link %d table %d\n", lm->id, (lm->is_vrf_slave ? lm->vrf : RT_TABLE_MAIN));
    return 0;
}

int insert_newroute(struct route_message *rm)
{
    const size_t IP_BUF_SIZE = 20;
	char addr[IP_BUF_SIZE], gw[IP_BUF_SIZE];

    inet_ntop(AF_INET, &(rm->addr), addr, IP_BUF_SIZE);
    inet_ntop(AF_INET, &(rm->gw), gw, IP_BUF_SIZE);

    printf("INFO: new route %s/%d dev %d gw %s table %d\n",
        addr, rm->prefix_len, rm->dev, gw, rm->vrf);

    return 0;
}
int insert_delroute(struct route_message *rm)
{
    const size_t IP_BUF_SIZE = 20;
	char addr[IP_BUF_SIZE], gw[IP_BUF_SIZE];

    inet_ntop(AF_INET, &(rm->addr), addr, IP_BUF_SIZE);
    inet_ntop(AF_INET, &(rm->gw), gw, IP_BUF_SIZE);

    printf("INFO: del route %s/%d dev %d gw %s table %d\n",
        addr, rm->prefix_len, rm->dev, gw, rm->vrf);

    return 0;
}

int insert_newneigh(struct neigh_message *nm)
{
    const size_t IP_BUF_SIZE = 20;
	char gw[IP_BUF_SIZE];

    inet_ntop(AF_INET, &(nm->gw), gw, IP_BUF_SIZE);

    printf("INFO: new neigh %02x:%02x:%02x:%02x:%02x:%02x gw %s\n",
        nm->addr[0], nm->addr[1], nm->addr[2], nm->addr[3], nm->addr[4], nm->addr[5], gw);

    return 0;
}
int insert_delneigh(struct neigh_message *nm)
{
    const size_t IP_BUF_SIZE = 20;
	char gw[IP_BUF_SIZE];

    inet_ntop(AF_INET, &(nm->gw), gw, IP_BUF_SIZE);

    printf("INFO: del neigh %02x:%02x:%02x:%02x:%02x:%02x gw %s\n",
        nm->addr[0], nm->addr[1], nm->addr[2], nm->addr[3], nm->addr[4], nm->addr[5], gw);

    return 0;
}
