control egress(inout headers hdr,
               inout metadata_t meta,
               in    psa_egress_input_metadata_t  istd,
               inout psa_egress_output_metadata_t ostd)
{
    action forward_action() {
        ostd.drop = false;
        ostd.clone = false;
    }
    apply {
        forward_action();
    }
}
