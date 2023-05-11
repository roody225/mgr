#!/bin/bash

#nikss-ctl table add pipe 0 ingress_vrf_table action name ingress_set_vrf key 3 data 254

#nikss-ctl table add pipe 0 ingress_routing_table_ip4 action name ingress_forward_ip4 key 254 10.0.0.0/24 data 4 10.0.0.2

#nikss-ctl table add pipe 0 ingress_arp_resolve_ip4 action name ingress_set_dst_mac key 10.0.0.2 data 00:00:01:02:03:04

ip addr add 192.168.1.1/24 dev s1-eth3
ip addr add 192.168.2.1/24 dev s1-eth4

ip r del 192.168.1.0/24
ip r del 192.168.2.0/24

ip r add 192.168.1.0/24 dev s1-eth3 via 192.168.1.1
ip r add 192.168.2.0/24 dev s1-eth4 via 192.168.2.1

ip neigh add 192.168.1.1 lladdr 11:22:33:44:55:66 dev s1-eth3
ip neigh add 192.168.2.1 lladdr 10:21:32:43:54:65 dev s1-eth4
