#!/usr/bin/python3

from scapy.all import *

x = Ether(dst='00:00:00:00:00:00',src='00:00:00:00:00:01')/IP(dst='192.168.2.15',src='10.0.0.1')/UDP(dport=12345,sport=54321)/'111111111111111111'
sendp(x, iface='eth0')
