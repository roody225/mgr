#include <stdio.h>
#include <string.h>

#include <linux/rtnetlink.h>
#include <arpa/inet.h>

#include "p4_handlers.h"
#include "nikss_helper.h"

int insert_newlink(struct link_message *lm)
{
    struct entry_spec spec;
    struct match_elem key;
    struct data_elem action_data;
    printf("INFO: new link %d table %d lladdr %02x:%02x:%02x:%02x:%02x:%02x\n",
        lm->id, (lm->is_vrf_slave ? lm->vrf : RT_TABLE_MAIN), lm->mac_addr[0],
        lm->mac_addr[1], lm->mac_addr[2], lm->mac_addr[3], lm->mac_addr[4], lm->mac_addr[5]);

    spec.pipe = 0;
    spec.table_name = "ingress_vrf_table";
    spec.action_name = "ingress_set_vrf";
    spec.elems = &key;
    spec.num_elems = 1;
    spec.data = &action_data;
    spec.num_data = 1;

    key.val = lm->id;
    key.type = U_32;

    action_data.val = lm->is_vrf_slave ? lm->vrf : RT_TABLE_MAIN;
    action_data.type = U_32_DATA;

    table_entry_add(&spec);

    spec.pipe = 0;
    spec.table_name = "ingress_src_mac_resolve_ip4";
    spec.action_name = "ingress_set_src_mac";
    spec.elems = &key;
    spec.num_elems = 1;
    spec.data = &action_data;
    spec.num_data = 1;

    key.val = lm->id;
    key.type = U_32;

    action_data.mac[0] = lm->mac_addr[5];
    action_data.mac[1] = lm->mac_addr[4];
    action_data.mac[2] = lm->mac_addr[3];
    action_data.mac[3] = lm->mac_addr[2];
    action_data.mac[4] = lm->mac_addr[1];
    action_data.mac[5] = lm->mac_addr[0];
    action_data.type = MAC_DATA;

    table_entry_add(&spec);

    return 0;
}
int insert_dellink(struct link_message *lm)
{
    printf("INFO: del link %d table %d lladdr %02x:%02x:%02x:%02x:%02x:%02x\n",
        lm->id, (lm->is_vrf_slave ? lm->vrf : RT_TABLE_MAIN), lm->mac_addr[0],
        lm->mac_addr[1], lm->mac_addr[2], lm->mac_addr[3], lm->mac_addr[4], lm->mac_addr[5]);
    return 0;
}

int insert_newroute(struct route_message *rm)
{
    struct entry_spec spec;
    struct match_elem key[2];
    struct data_elem action_data[2];
    const size_t IP_BUF_SIZE = 20;
	char addr[IP_BUF_SIZE], gw[IP_BUF_SIZE];

    inet_ntop(AF_INET, &(rm->addr), addr, IP_BUF_SIZE);
    inet_ntop(AF_INET, &(rm->gw), gw, IP_BUF_SIZE);

    printf("INFO: new route %s/%d dev %d gw %s table %d\n",
        addr, rm->prefix_len, rm->dev, gw, rm->vrf);

    spec.pipe = 0;
    spec.elems = key;
    spec.data = action_data;
    if (rm->vrf == RT_TABLE_LOCAL) {
        spec.table_name = "ingress_forward_local_table_ip4";
        spec.action_name = "ingress_normal_path_to_kernel";
        spec.num_elems = 1;
        spec.num_data = 0;

        key[0].val = htonl(rm->addr);
        key[0].type = IP4_ADDR;
    } else if (rm->dev == 0) {
        return 0;
    } else {
        spec.table_name = "ingress_routing_table_ip4";
        spec.action_name = "ingress_forward_ip4";
        spec.num_elems = 2;
        spec.num_data = 2;

        key[0].val = rm->vrf;
        key[0].type = U_32;
        key[1].val = htonl(rm->addr);
        key[1].prefix_len = rm->prefix_len;
        key[1].type = IP4_LPM;

        action_data[0].val = rm->dev;
        action_data[0].type = U_32_DATA;
        action_data[1].val = htonl(rm->gw);
        action_data[1].type = IP4_DATA;
    }

    table_entry_add(&spec);
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
    struct entry_spec spec;
    struct match_elem key;
    struct data_elem action_data;
    const size_t IP_BUF_SIZE = 20;
	char gw[IP_BUF_SIZE];

    inet_ntop(AF_INET, &(nm->gw), gw, IP_BUF_SIZE);

    printf("INFO: new neigh %02x:%02x:%02x:%02x:%02x:%02x gw %s\n",
        nm->addr[0], nm->addr[1], nm->addr[2], nm->addr[3], nm->addr[4], nm->addr[5], gw);

    spec.pipe = 0;
    spec.table_name = "ingress_arp_resolve_ip4";
    spec.action_name = "ingress_set_dst_mac";
    spec.elems = &key;
    spec.num_elems = 1;
    spec.data = &action_data;
    spec.num_data = 1;

    key.val = htonl(nm->gw);
    key.type = IP4_ADDR;

    action_data.mac[0] = nm->addr[5];
    action_data.mac[1] = nm->addr[4];
    action_data.mac[2] = nm->addr[3];
    action_data.mac[3] = nm->addr[2];
    action_data.mac[4] = nm->addr[1];
    action_data.mac[5] = nm->addr[0];
    action_data.type = MAC_DATA;

    table_entry_add(&spec);
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
