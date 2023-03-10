#!/bin/bash

nikss-ctl table add pipe 0 ingress_vrf_table action name ingress_set_vrf key 3 data 254

nikss-ctl table add pipe 0 ingress_routing_table_ip4 action name ingress_forward_ip4 key 254 10.0.0.0/24 data 4 10.0.0.2

nikss-ctl table add pipe 0 ingress_arp_resolve_ip4 action name ingress_set_dst_mac key 10.0.0.2 data 00:00:01:02:03:04
