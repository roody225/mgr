control ingress(inout headers hdr,
                inout metadata_t meta,
                in    psa_ingress_input_metadata_t  istd,
                inout psa_ingress_output_metadata_t ostd)
{
    action setup_action() {
        meta.process = PROCESS_GO;
        ostd.clone = false;
    }

    action set_vrf(bit<32> vrf) {
        meta.vrf = vrf;
    }

    action set_default_vrf() {
        meta.vrf = MAIN_VRF;
    }

    table vrf_table {
        key = {
            istd.ingress_port: exact;
        }
        actions = {
            set_vrf;
            set_default_vrf;
        }
        default_action = set_default_vrf();
        size = 1024;
    }

    action normal_path_to_kernel() {
        ostd.drop = false;
        ostd.egress_port = (PortId_t)0;

        meta.process = PROCESS_STOP;
    }

    action no_action() {
        // do nothing
    }

    table forward_local_table_ip4 {
        key = {
            hdr.ip4.dstAddr: exact;
        }
        actions = {
            normal_path_to_kernel;
            no_action;
        }
        default_action = no_action();
        size = 1024;
    }

    // table forward_local_table_ip6 {
    //     key = {
    //         hdr.ip6.dstAddr: exact;
    //     }
    //     actions = {
    //         normal_path_to_kernel;
    //         no_action;
    //     }
    //     default_action = no_action();
    //     size = 1024;
    // }

    action forward_ip4(PortId_t port, bit<32> via) {
        if (via != 0) {
            meta.viaIP4 = via;
        } else {
            meta.viaIP4 = 0;
        }

        ostd.drop = false;
        ostd.egress_port = port;
    }

    table routing_table_ip4 {
        key = {
            meta.vrf: exact;
            hdr.ip4.dstAddr: lpm;
        }
        actions = {
            normal_path_to_kernel;
            forward_ip4;
        }
        default_action = normal_path_to_kernel();
        size = 1024;
    }

    // action forward_ip6(PortId_t port, bit<128> via) {
    //     if (via == 0) {
    //         meta.viaIP6 = hdr.ip6.dstAddr;
    //     } else {
    //         meta.viaIP6 = via;
    //     }

    //     ostd.drop = false;
    //     ostd.egress_port = port;
    // }

    // table routing_table_ip6 {
    //     key = {
    //         meta.vrf: exact;
    //         hdr.ip6.dstAddr: lpm;
    //     }
    //     actions = {
    //         normal_path_to_kernel;
    //         forward_ip6;
    //     }
    //     default_action = normal_path_to_kernel();
    //     size = 1024;
    // }

    action set_dst_mac(bit<48> dst_mac) {
        hdr.ethernet.dst = dst_mac;
    }

    table arp_resolve_ip4 {
        key = {
            meta.viaIP4: exact;
        }
        actions = {
            normal_path_to_kernel;
            set_dst_mac;
        }
        default_action = normal_path_to_kernel();
        size = 1024;
    }

    // table arp_resolve_ip6 {
    //     key = {
    //         meta.viaIP6: exact;
    //     }
    //     actions = {
    //         normal_path_to_kernel;
    //         set_dst_mac;
    //     }
    //     default_action = normal_path_to_kernel();
    //     size = 1024;
    // }

    apply {
        if (!hdr.ip4.isValid() /* && !hdr.ip6.isValid() */) {
            normal_path_to_kernel();
        } else {
            setup_action();
            vrf_table.apply();

            forward_local_table_ip4.apply();
            if (meta.process == PROCESS_GO) {
                routing_table_ip4.apply();
            }
            if ((meta.process == PROCESS_GO) && (meta.viaIP4 != 0)) {
                arp_resolve_ip4.apply();
            }
            // if (hdr.ip4.isValid()) {
            //     forward_local_table_ip4.apply();
            //     if (meta.process == PROCESS_GO) {
            //         routing_table_ip4.apply();
            //     }
            //     if (meta.process == PROCESS_GO) {
            //         arp_resolve_ip4.apply();
            //     }
            // } else {
            //     forward_local_table_ip6.apply();
            //     if (meta.process == PROCESS_GO) {
            //         routing_table_ip6.apply();
            //     }
            //     if (meta.process == PROCESS_GO) {
            //         arp_resolve_ip6.apply();
            //     }
            // }
        }
    }
}
