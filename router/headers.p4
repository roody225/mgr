const bit<16> HDR_ETH_TYPE_IP4 = 0x0800;
const bit<16> HDR_ETH_TYPE_IP6 = 0x86DD;

struct metadata_t {}

struct empty_t {}

header ethernet_t {
    bit<48> dst;
    bit<48> src;
    bit<16> etherType;
}

header ip4_t {
    bit<4>  version;
    bit<4>  ihl;
    bit<6>  dscp;
    bit<2>  ecn;
    bit<16> len;
    bit<16> id;
    bit<3>  flags;
    bit<13> fragOffset
    bit<8>  ttl;
    bit<8>  protocol;
    bit<16> checksum;
    bit<32> srcAddr;
    bit<32> dstAddr;
}

header ip6_t {
    bit<4>   version;
    bit<8>   trafficClass;
    bit<20>  flowLabel;
    bit<16>  len;
    bit<8>   nextHdr;
    bit<8>   hopLimit;
    bit<128> srcAddr;
    bit<128> dstAddr;
}
