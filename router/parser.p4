#include "headers.p4"

struct headers {
    ethernet_t ethernet;
    ip4_t ip4;
    ip6_t ip6;
}

parser IngressParserImpl(packet_in buffer,
                         out headers parsed_hdr,
                         inout metadata_t meta,
                         in psa_ingress_parser_input_metadata_t istd,
                         in empty_t resubmit_meta,
                         in empty_t recirculate_meta) {
    state start {
        buffer.extract(parsed_hdr.ethernet);
        transition select(parsed_hdr.ethernet.etherType) {
            HDR_ETH_TYPE_IP4: parse_ip4;
            HDR_ETH_TYPE_IP6: parse_ip6;
            default: accept;
        }
    }
    state parse_ip4 {
        buffer.extract(parsed_hdr.ip4);
        transition accept;
    }
    state parse_ip6 {
        buffer.extract(parsed_hdr.ip6);
        transition accept;
    }
}

parser EgressParserImpl(packet_in buffer,
                        out headers parsed_hdr,
                        inout metadata_t meta,
                        in psa_egress_parser_input_metadata_t istd,
                        in empty_t normal_meta,
                        in empty_t clone_i2e_meta,
                        in empty_t clone_e2e_meta) {
    state start {
        transition accept;
    }
}
