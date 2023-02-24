#include "headers.p4"

struct headers {}

parser IngressParserImpl(packet_in buffer,
                         out headers parsed_hdr,
                         inout metadata_t meta,
                         in psa_ingress_parser_input_metadata_t istd,
                         in empty_t resubmit_meta,
                         in empty_t recirculate_meta) {
    state start {
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
