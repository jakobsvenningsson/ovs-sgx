/*
 * meta-flow.c
 *
 *  Created on: May 7, 2018
 *      Author: jorge
 */

#include "meta-flow.h"
#include "sgx_htons.h"
#include "util.h"

/* Returns true if 'flow' meets the prerequisites for 'mf', false otherwise. */

bool
mf_are_prereqs_ok(const struct mf_field *mf, const struct flow *flow)
{
    switch (mf->prereqs) {
    case MFP_NONE:
        return true;

    case MFP_ARP:
      return (flow->dl_type == htons(ETH_TYPE_ARP) ||
              flow->dl_type == htons(ETH_TYPE_RARP));
    case MFP_IPV4:
        return flow->dl_type == htons(ETH_TYPE_IP);
    case MFP_IPV6:
        return flow->dl_type == htons(ETH_TYPE_IPV6);
    case MFP_VLAN_VID:
        return (flow->vlan_tci & htons(VLAN_CFI)) != 0;
    case MFP_MPLS:
        return eth_type_mpls(flow->dl_type);
    case MFP_IP_ANY:
        return is_ip_any(flow);

    case MFP_TCP:
        return is_ip_any(flow) && flow->nw_proto == IPPROTO_TCP;
    case MFP_UDP:
        return is_ip_any(flow) && flow->nw_proto == IPPROTO_UDP;
    case MFP_ICMPV4:
        return is_icmpv4(flow);
    case MFP_ICMPV6:
        return is_icmpv6(flow);

    case MFP_ND:
        return (is_icmpv6(flow)
                && flow->tp_dst == htons(0)
                && (flow->tp_src == htons(ND_NEIGHBOR_SOLICIT) ||
                    flow->tp_src == htons(ND_NEIGHBOR_ADVERT)));
    case MFP_ND_SOLICIT:
        return (is_icmpv6(flow)
                && flow->tp_dst == htons(0)
                && (flow->tp_src == htons(ND_NEIGHBOR_SOLICIT)));
    case MFP_ND_ADVERT:
        return (is_icmpv6(flow)
                && flow->tp_dst == htons(0)
                && (flow->tp_src == htons(ND_NEIGHBOR_ADVERT)));
    }

    NOT_REACHED();
}

/* Copies the value of field 'mf' from 'flow' into 'value'.  The caller is
 * responsible for ensuring that 'flow' meets 'mf''s prerequisites. */
void
mf_get_value(const struct mf_field *mf, const struct flow *flow,
             union mf_value *value)
{
    switch (mf->id) {
    case MFF_TUN_ID:
        value->be64 = flow->tunnel.tun_id;
        break;
    case MFF_TUN_SRC:
        value->be32 = flow->tunnel.ip_src;
        break;
    case MFF_TUN_DST:
        value->be32 = flow->tunnel.ip_dst;
        break;
    case MFF_TUN_FLAGS:
        value->be16 = htons(flow->tunnel.flags);
        break;
    case MFF_TUN_TTL:
        value->u8 = flow->tunnel.ip_ttl;
        break;
    case MFF_TUN_TOS:
        value->u8 = flow->tunnel.ip_tos;
        break;

    case MFF_METADATA:
        value->be64 = flow->metadata;
        break;

    case MFF_IN_PORT:
        value->be16 = htons(flow->in_port);
        break;

    case MFF_SKB_PRIORITY:
        value->be32 = htonl(flow->skb_priority);
        break;

    case MFF_SKB_MARK:
        value->be32 = htonl(flow->skb_mark);
        break;

    CASE_MFF_REGS:
        value->be32 = htonl(flow->regs[mf->id - MFF_REG0]);
        break;

    case MFF_ETH_SRC:
        memcpy(value->mac, flow->dl_src, ETH_ADDR_LEN);
        break;

    case MFF_ETH_DST:
        memcpy(value->mac, flow->dl_dst, ETH_ADDR_LEN);
        break;

    case MFF_ETH_TYPE:
        value->be16 = flow->dl_type;
        break;

    case MFF_VLAN_TCI:
        value->be16 = flow->vlan_tci;
        break;

    case MFF_DL_VLAN:
        value->be16 = flow->vlan_tci & htons(VLAN_VID_MASK);
        break;
    case MFF_VLAN_VID:
        value->be16 = flow->vlan_tci & htons(VLAN_VID_MASK | VLAN_CFI);
        break;

    case MFF_DL_VLAN_PCP:
    case MFF_VLAN_PCP:
        value->u8 = vlan_tci_to_pcp(flow->vlan_tci);
        break;

    case MFF_MPLS_LABEL:
        value->be32 = htonl(mpls_lse_to_label(flow->mpls_lse));
        break;

    case MFF_MPLS_TC:
        value->u8 = mpls_lse_to_tc(flow->mpls_lse);
        break;

    case MFF_MPLS_BOS:
        value->u8 = mpls_lse_to_bos(flow->mpls_lse);
        break;

    case MFF_IPV4_SRC:
        value->be32 = flow->nw_src;
        break;

    case MFF_IPV4_DST:
        value->be32 = flow->nw_dst;
        break;

    case MFF_IPV6_SRC:
        value->ipv6 = flow->ipv6_src;
        break;

    case MFF_IPV6_DST:
        value->ipv6 = flow->ipv6_dst;
        break;

    case MFF_IPV6_LABEL:
        value->be32 = flow->ipv6_label;
        break;

    case MFF_IP_PROTO:
        value->u8 = flow->nw_proto;
        break;

    case MFF_IP_DSCP:
        value->u8 = flow->nw_tos & IP_DSCP_MASK;
        break;

    case MFF_IP_DSCP_SHIFTED:
        value->u8 = flow->nw_tos >> 2;
        break;

    case MFF_IP_ECN:
        value->u8 = flow->nw_tos & IP_ECN_MASK;
        break;

    case MFF_IP_TTL:
        value->u8 = flow->nw_ttl;
        break;

    case MFF_IP_FRAG:
        value->u8 = flow->nw_frag;
        break;

    case MFF_ARP_OP:
        value->be16 = htons(flow->nw_proto);
        break;

    case MFF_ARP_SPA:
        value->be32 = flow->nw_src;
        break;

    case MFF_ARP_TPA:
        value->be32 = flow->nw_dst;
        break;

    case MFF_ARP_SHA:
    case MFF_ND_SLL:
        memcpy(value->mac, flow->arp_sha, ETH_ADDR_LEN);
        break;

    case MFF_ARP_THA:
    case MFF_ND_TLL:
        memcpy(value->mac, flow->arp_tha, ETH_ADDR_LEN);
        break;

    case MFF_TCP_SRC:
    case MFF_UDP_SRC:
        value->be16 = flow->tp_src;
        break;

    case MFF_TCP_DST:
    case MFF_UDP_DST:
        value->be16 = flow->tp_dst;
        break;

    case MFF_ICMPV4_TYPE:
    case MFF_ICMPV6_TYPE:
        value->u8 = ntohs(flow->tp_src);
        break;

    case MFF_ICMPV4_CODE:
    case MFF_ICMPV6_CODE:
        value->u8 = ntohs(flow->tp_dst);
        break;

    case MFF_ND_TARGET:
        value->ipv6 = flow->nd_target;
        break;

    case MFF_N_IDS:
    default:
        NOT_REACHED();
    }

}
