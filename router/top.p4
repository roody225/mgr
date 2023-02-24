#include <core.p4>
#include <psa.p4>

#include "parser.p4"
#include "ingress.p4"
#include "egress.p4"
#include "deparser.p4"

IngressPipeline(IngressParserImpl(),
                ingress(),
                IngressDeparserImpl()) ip;

EgressPipeline(EgressParserImpl(),
               egress(),
               EgressDeparserImpl()) ep;

PSA_Switch(ip, PacketReplicationEngine(), ep, BufferingQueueingEngine()) main;
