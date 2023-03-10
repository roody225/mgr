#!/usr/bin/env python3

import socket

from lib.nikss_mn import P4Host, NIKSSSwitch

from mininet.net import Mininet
from mininet.topo import Topo
from mininet.log import setLogLevel, info
from mininet.cli import CLI
from mininet.topo import SingleSwitchTopo

from time import sleep

class MyCustomTopo(Topo):

    def __init__(self, **opts):
        # Initialize topology and default options
        Topo.__init__(self, **opts)

        switch = self.addSwitch('s1',
                                bpf_path="p4_router.o",
                                enable_tracing = True)
        for h in range(3):
            host = self.addHost('h%d' % (h + 1),
                                mac = '00:04:00:00:00:%02x' % (h + 1))
            self.addLink(host, switch, (h + 1), (h + 2))


def main():
    topo = MyCustomTopo()
    net = Mininet(topo = topo,
                  host = P4Host,
                  switch = NIKSSSwitch,
                  controller = None)
    net.start()

    sleep(1)

    h1 = net.get('h1')
    h1.setARP("10.0.0.2", "00:04:00:00:00:02")
    h1.setARP("10.0.0.3", "00:04:00:00:00:03")
    h1_port_id = socket.if_nametoindex("s1-eth2")

    h2 = net.get('h2')
    h2.setARP("10.0.0.1", "00:04:00:00:00:01")
    h1.setARP("10.0.0.3", "00:04:00:00:00:03")
    h2_port_id = socket.if_nametoindex("s1-eth3")

    h3 = net.get('h3')
    h3.setARP("10.0.0.1", "00:04:00:00:00:01")
    h1.setARP("10.0.0.2", "00:04:00:00:00:02")
    h3_port_id = socket.if_nametoindex("s1-eth4")

    s1 = net.get('s1')

    CLI( net )
    net.stop()


if __name__ == '__main__':
    setLogLevel( 'info' )
    main()
