#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#include <linux/rtnetlink.h>
#include <libmnl/libmnl.h>
#include <poll.h>

#include "offload.h"

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
    printf("INFO: handling netlink message type: %d\n", h->nlmsg_type);
    return MNL_CB_OK;
}

void *listen_loop(void *arg __attribute__((unused)))
{
    uint8_t buf[MNL_SOCKET_BUFFER_SIZE];
    ssize_t bytes_received;
    struct pollfd fds;

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

        printf("INFO: netlink message received %ld bytes\n", bytes_received);

        switch (mnl_cb_run(buf, bytes_received, 0, 0, handle_message, NULL)) {
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
