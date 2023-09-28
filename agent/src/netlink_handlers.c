#include <libmnl/libmnl.h>
#include <linux/rtnetlink.h>

#include <string.h>

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
    struct link_message *lm = (struct link_message *)data;
    unsigned type = mnl_attr_get_type(attr);

    switch (type) {
        case IFLA_LINKINFO:
            mnl_attr_parse_nested(attr, parse_link_info_callback, data);
            break;
        case IFLA_ADDRESS:
            if (mnl_attr_get_payload_len(attr) != 6)
                break;
            memcpy(lm->mac_addr, mnl_attr_get_payload(attr), ETH_ALEN);
            break;
    }

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
            if (rm->is_ip4)
                rm->addr = mnl_attr_get_u32(attr);
            else
                memcpy(rm->addr6, mnl_attr_get_payload(attr), IPV6_ADDR_LEN);
            break;
        case RTA_GATEWAY:
            if (rm->is_ip4)
                rm->gw = mnl_attr_get_u32(attr);
            else
                memcpy(rm->gw6, mnl_attr_get_payload(attr), IPV6_ADDR_LEN);
            break;
    }

    return MNL_CB_OK;
}

int handle_newroute(const struct nlmsghdr *h, struct route_message *rm)
{
    struct rtmsg *rtm = (struct rtmsg *)mnl_nlmsg_get_payload(h);

    if (h == NULL)
        return MNL_CB_ERROR;

    switch (rtm->rtm_family) {
        case AF_INET:
            rm->is_ip4 = 1;
            break;
        case AF_INET6:
            rm->is_ip4 = 0;
            break;
        default:
            return MNL_CB_OK;
    }

    rm->vrf = rtm->rtm_table;
    rm->prefix_len = rtm->rtm_dst_len;

    return mnl_attr_parse(h, sizeof(struct rtmsg), parse_route_callback, rm);
}

int handle_delroute(const struct nlmsghdr *h, struct route_message *rm)
{
    struct rtmsg *rtm = (struct rtmsg *)mnl_nlmsg_get_payload(h);

    if (h == NULL)
        return MNL_CB_ERROR;

    switch (rtm->rtm_family) {
        case AF_INET:
            rm->is_ip4 = 1;
            break;
        case AF_INET6:
            rm->is_ip4 = 0;
            break;
        default:
            return MNL_CB_OK;
    }

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
            if (nm->is_ip4)
                nm->gw = mnl_attr_get_u32(attr);
            else
                memcpy(nm->gw6, mnl_attr_get_payload(attr), IPV6_ADDR_LEN);
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

    switch (nd->ndm_family) {
        case AF_INET:
            nm->is_ip4 = 1;
            break;
        case AF_INET6:
            nm->is_ip4 = 0;
            break;
        default:
            return MNL_CB_OK;
    }

    nm->state = nd->ndm_state;

    return mnl_attr_parse(h, sizeof(struct ndmsg), parse_neigh_callback, nm);
}
int handle_delneigh(const struct nlmsghdr *h, struct neigh_message *nm)
{
    struct ndmsg *nd = (struct ndmsg *)mnl_nlmsg_get_payload(h);

    if (h == NULL)
        return MNL_CB_ERROR;

    switch (nd->ndm_family) {
        case AF_INET:
            nm->is_ip4 = 1;
            break;
        case AF_INET6:
            nm->is_ip4 = 0;
            break;
        default:
            return MNL_CB_OK;
    }

    nm->state = nd->ndm_state;

    return mnl_attr_parse(h, sizeof(struct ndmsg), parse_neigh_callback, nm);
}
