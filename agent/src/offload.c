#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#include <linux/rtnetlink.h>
#include <libmnl/libmnl.h>
#include <poll.h>

#include "offload.h"
#include "netlink_handlers.h"
#include "p4_handlers.h"

static struct mnl_socket *netlink_socket = NULL;
static bool process = false;
static pthread_mutex_t netlink_mutex;

int netlink_connect()
{
    if (netlink_socket != NULL) {
        fprintf(stderr, "ERROR: attempt to establish more than one netlink connection\n");
        return -1;
    }

    netlink_socket = mnl_socket_open(NETLINK_ROUTE);
    if (netlink_socket == NULL) {
        fprintf(stderr, "ERROR: cannot open netlink socket\n");
        return -1;
    }

    if (mnl_socket_bind(netlink_socket, MULTICAST_GROUPS, MNL_SOCKET_AUTOPID) < 0) {
        mnl_socket_close(netlink_socket);
        fprintf(stderr, "ERROR: cannot bind netlink socket\n");
        return -1;
    }

    return 0;
}

void netlink_disconnect()
{
    if (netlink_socket == NULL) {
        fprintf(stderr, "WARNING: attempt to close uninitialized netlink connection\n");
        return;
    }

    mnl_socket_close(netlink_socket);
}

static int handle_message(const struct nlmsghdr *h, void *data)
{
    struct netlink_info *ni = (struct netlink_info *)data;
    int ret = MNL_CB_ERROR;

    memset(ni, 0, sizeof(struct netlink_info));

    switch (h->nlmsg_type) {
        case RTM_NEWLINK:
            ni->type = RTM_NEWLINK;
            ret = handle_newlink(h, &(ni->lm));
            if (ret == MNL_CB_OK) {
                if (insert_newlink(&(ni->lm)))
                    fprintf(stderr, "ERROR: failed to insert newlink\n");
            }
            break;
        case RTM_DELLINK:
            ni->type = RTM_DELLINK;
            ret = handle_dellink(h, &(ni->lm));
            if (ret == MNL_CB_OK) {
                if (insert_dellink(&(ni->lm)))
                    fprintf(stderr, "ERROR: failed to insert dellink\n");
            }
            break;
        case RTM_NEWROUTE:
            ni->type = RTM_NEWROUTE;
            ret = handle_newroute(h, &(ni->rm));
            if (ret == MNL_CB_OK) {
                if (insert_newroute(&(ni->rm)))
                    fprintf(stderr, "ERROR: failed to insert newroute\n");
            }
            break;
        case RTM_DELROUTE:
            ni->type = RTM_DELROUTE;
            ret = handle_delroute(h, &(ni->rm));
            if (ret == MNL_CB_OK) {
                if (insert_delroute(&(ni->rm)))
                    fprintf(stderr, "ERROR: failed to insert delroute\n");
            }
            break;
        case RTM_NEWNEIGH:
            ni->type = RTM_NEWNEIGH;
            ret = handle_newneigh(h, &(ni->nm));
            if (ret == MNL_CB_OK) {
                if ((ni->nm.state & ACTIVE_NEIGH_STATES_MASK) == 0) {
                    if (insert_delneigh(&(ni->nm)))
                        fprintf(stderr, "ERROR: failed to insert delneigh\n");
                } else {
                    if (insert_newneigh(&(ni->nm)))
                        fprintf(stderr, "ERROR: failed to insert newneigh\n");
                }
            }
            break;
        case RTM_DELNEIGH:
            ni->type = RTM_DELNEIGH;
            ret = handle_delneigh(h, &(ni->nm));
            if (ret == MNL_CB_OK) {
                if (insert_delneigh(&(ni->nm)))
                    fprintf(stderr, "ERROR: failed to insert delneigh\n");
            }
            break;
        default:
            ni->type = RTM_MAX;
            ret = MNL_CB_OK;
            break;
    }

    return ret;
}

void *listen_loop(void *arg __attribute__((unused)))
{
    uint8_t buf[MNL_SOCKET_BUFFER_SIZE];
    ssize_t bytes_received;
    struct pollfd fds;
    struct netlink_info ni;

    if (pthread_mutex_init(&netlink_mutex, NULL) != 0) {
        fprintf(stderr, "ERROR: netlink mutex init failed\n");
        return NULL;
    }

    if (netlink_socket == NULL) {
        fprintf(stderr, "ERROR: netlink socket not opened\n");
        return NULL;
    }

    fds.fd = mnl_socket_get_fd(netlink_socket);
    fds.events = POLLIN;

    process = true;
    while (process) {
        switch (poll(&fds, 1, 1000)) {
            case 0:
                continue;
            case 1:
                break;
            default:
                fprintf(stderr, "ERROR: poll error: %s\n", strerror(errno));
                process = false;
                continue;
        }

        bytes_received = mnl_socket_recvfrom(netlink_socket, buf, MNL_SOCKET_BUFFER_SIZE);
        if (bytes_received < 0) {
            if ((errno == EINTR) || (errno == EAGAIN) || (errno == EWOULDBLOCK))
                continue;

            fprintf(stderr, "ERROR: recv error\n");
            break;
        }

        switch (mnl_cb_run(buf, bytes_received, 0, 0, handle_message, &ni)) {
            case MNL_CB_OK:
                break;
            case MNL_CB_STOP:
                pthread_mutex_unlock(&netlink_mutex);
                break;
            default:
                fprintf(stderr, "ERROR: message proccessing failure\n");
                process = false;
                continue;
        }
    }

    pthread_mutex_destroy(&netlink_mutex);

    return NULL;
}

void wait_for_listen_loop()
{
    // yes it is intended to have spin lock here
    do {} while(!process);
}

void listen_loop_stop()
{
    process = false;
}

void send_netlink_message(uint16_t type, void *data, size_t data_size, uint16_t flags)
{
    static uint32_t sequence_number = 0;
    uint8_t buf[MNL_SOCKET_BUFFER_SIZE];
    struct nlmsghdr *nlh = mnl_nlmsg_put_header(buf);

    if (netlink_socket == NULL) {
        fprintf(stderr, "ERROR: netlink socket must be opened before sending message\n");
        return;
    }

    if (!process) {
        fprintf(stderr, "ERROR: netlink listener must be run before sending message\n");
        return;
    }

    nlh->nlmsg_flags = flags;
    nlh->nlmsg_type = type;
    nlh->nlmsg_seq = sequence_number++;

    if (data != NULL)
        memcpy(mnl_nlmsg_put_extra_header(nlh, data_size), data, data_size);

    pthread_mutex_lock(&netlink_mutex);
    if (mnl_socket_sendto(netlink_socket, nlh, nlh->nlmsg_len) < 0)
        fprintf(stderr, "ERROR: cannot send netlink request\n");
}
