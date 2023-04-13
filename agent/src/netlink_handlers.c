#include <libmnl/libmnl.h>
#include <linux/rtnetlink.h>

#include <string.h>
#include <stdio.h>

#include "netlink_handlers.h"

static int parse_link_info_data_vrf_callback(const struct nlattr *attr, void *data)
{
    struct link_message *lm = (struct link_message *)data;

    if (mnl_attr_get_type(attr) == IFLA_VRF_TABLE)
            lm->vrf = mnl_attr_get_u32(attr);

    return MNL_CB_OK;
}

static int parse_link_info_callback(const struct nlattr *attr, void *data)
{
    struct link_message *lm = (struct link_message *)data;
    uint16_t type = mnl_attr_get_type(attr);

    switch (type) {
        case IFLA_INFO_SLAVE_KIND:
            if ((strlen(mnl_attr_get_payload(attr)) != 3) ||
                (memcmp("vrf", mnl_attr_get_payload(attr), 3) != 0))
                return MNL_CB_OK;
            lm->is_vrf_slave = 1;
            break;
        case IFLA_INFO_SLAVE_DATA:
            if (lm->is_vrf_slave == 0)
                return MNL_CB_OK;
            return mnl_attr_parse_nested(attr, parse_link_info_data_vrf_callback, data);
            break;
    }

    return MNL_CB_OK;
}

static int parse_link_callback(const struct nlattr *attr, void *data)
{
    if (mnl_attr_get_type(attr) == IFLA_LINKINFO)
            mnl_attr_parse_nested(attr, parse_link_info_callback, data);

    return MNL_CB_OK;
}

int handle_newlink(const struct nlmsghdr *h, struct link_message *lm)
{
    if (h == NULL)
        return MNL_CB_ERROR;

    lm->id = ((struct ifinfomsg *)mnl_nlmsg_get_payload(h))->ifi_index;

    return mnl_attr_parse(h, sizeof(struct ifinfomsg), parse_link_callback, lm);
}
int handle_dellink(const struct nlmsghdr *h, struct link_message *lm)
{
    if (h == NULL)
        return MNL_CB_ERROR;

    lm->id = ((struct ifinfomsg *)mnl_nlmsg_get_payload(h))->ifi_index;

    return mnl_attr_parse(h, sizeof(struct ifinfomsg), parse_link_callback, lm);
}

static int parse_route_callback(const struct nlattr *attr, void *data)
{
    struct route_message *rm = (struct route_message *)data;
    unsigned type = mnl_attr_get_type(attr);

    switch (type) {
        case RTA_TABLE:
            rm->vrf = mnl_attr_get_u32(attr);
            break;
        case RTA_OIF:
            rm->dev = mnl_attr_get_u32(attr);
            break;
        case RTA_DST:
            rm->addr = mnl_attr_get_u32(attr);
            break;
        case RTA_GATEWAY:
            rm->gw = mnl_attr_get_u32(attr);
            break;
    }

    return MNL_CB_OK;
}

int handle_newroute(const struct nlmsghdr *h, struct route_message *rm)
{
    struct rtmsg *rtm = (struct rtmsg *)mnl_nlmsg_get_payload(h);

    if (h == NULL)
        return MNL_CB_ERROR;

    if (rtm->rtm_family != AF_INET)
        return MNL_CB_OK;

    rm->is_ip4 = 1;
    rm->vrf = rtm->rtm_table;
    rm->prefix_len = rtm->rtm_dst_len;

    return mnl_attr_parse(h, sizeof(struct rtmsg), parse_route_callback, rm);
}

int handle_delroute(const struct nlmsghdr *h, struct route_message *rm)
{
    struct rtmsg *rtm = (struct rtmsg *)mnl_nlmsg_get_payload(h);

    if (h == NULL)
        return MNL_CB_ERROR;

    if (rtm->rtm_family != AF_INET)
        return MNL_CB_OK;

    rm->is_ip4 = 1;
    rm->vrf = rtm->rtm_table;
    rm->prefix_len = rtm->rtm_dst_len;

    return mnl_attr_parse(h, sizeof(struct rtmsg), parse_route_callback, rm);
}

static int parse_neigh_callback(const struct nlattr *attr, void *data)
{
    struct neigh_message *nm = (struct neigh_message *)data;
    unsigned type = mnl_attr_get_type(attr);

    switch (type) {
        case NDA_DST:
            nm->gw = mnl_attr_get_u32(attr);
            break;
        case NDA_LLADDR:
            memcpy(nm->addr, mnl_attr_get_payload(attr), ETH_ALEN);
            break;
    }

    return MNL_CB_OK;
}

int handle_newneigh(const struct nlmsghdr *h, struct neigh_message *nm)
{
    struct ndmsg *nd = (struct ndmsg *)mnl_nlmsg_get_payload(h);

    if (h == NULL)
        return MNL_CB_ERROR;

    if (nd->ndm_family != AF_INET)
        return MNL_CB_OK;

    return mnl_attr_parse(h, sizeof(struct ndmsg), parse_neigh_callback, nm);
}
int handle_delneigh(const struct nlmsghdr *h, struct neigh_message *nm)
{
    struct ndmsg *nd = (struct ndmsg *)mnl_nlmsg_get_payload(h);

    if (h == NULL)
        return MNL_CB_ERROR;

    if (nd->ndm_family != AF_INET)
        return MNL_CB_OK;

    return mnl_attr_parse(h, sizeof(struct ndmsg), parse_neigh_callback, nm);
}
