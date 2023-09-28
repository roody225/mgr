#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>

#include <linux/rtnetlink.h>
#include <sys/socket.h>

#include "offload.h"

static void termination_handler(int sig __attribute__((unused)))
{
    printf("SIGINT or SIGTERM received, terminating\n");
    listen_loop_stop();
}

int main()
{
    pthread_t thread_id;
    struct ifinfomsg ifi = {0};
    struct rtmsg rtm = {0};
    struct ndmsg ndm = {0};

    signal(SIGINT, termination_handler);
    signal(SIGTERM, termination_handler);

    if (netlink_connect() != 0) {
        fprintf(stderr, "error occured while connecting to netlink\n");
        return -1;
    }

    pthread_create(&thread_id, NULL, listen_loop, NULL);
    wait_for_listen_loop();

    printf("INFO: netlink listener started\n");

    ifi.ifi_family = AF_INET;
    send_netlink_message(RTM_GETLINK, &ifi, sizeof(ifi), NLM_F_REQUEST | NLM_F_DUMP);

    rtm.rtm_family = AF_INET;
    send_netlink_message(RTM_GETROUTE, &rtm, sizeof(rtm), NLM_F_REQUEST | NLM_F_DUMP);

    rtm.rtm_family = AF_INET6;
    send_netlink_message(RTM_GETROUTE, &rtm, sizeof(rtm), NLM_F_REQUEST | NLM_F_DUMP);

    ndm.ndm_family = AF_INET;
    send_netlink_message(RTM_GETNEIGH, &ndm, sizeof(ndm), NLM_F_REQUEST | NLM_F_DUMP);

    ndm.ndm_family = AF_INET6;
    send_netlink_message(RTM_GETNEIGH, &ndm, sizeof(ndm), NLM_F_REQUEST | NLM_F_DUMP);

    pthread_join(thread_id, NULL);

    netlink_disconnect();

    return 0;
}
