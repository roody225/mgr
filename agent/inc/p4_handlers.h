#ifndef _P4_HANDLERS_H_
#define _P4_HANDLERS_H_

#include "netlink_handlers.h"

int insert_newlink(struct link_message *lm);
int insert_dellink(struct link_message *lm);

int insert_newroute(struct route_message *rm);
int insert_delroute(struct route_message *rm);

int insert_newneigh(struct neigh_message *nm);
int insert_delneigh(struct neigh_message *nm);

#endif /* _P4_HANDLERS_H_ */
