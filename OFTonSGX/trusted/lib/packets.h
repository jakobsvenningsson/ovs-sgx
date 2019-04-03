/*
 * packets.h
 *
 *  Created on: Apr 23, 2018
 *      Author: jorge
 */

#ifndef ENCLAVE_MYENCLAVE_TRUSTED_LIB_PACKETS_H_
#define ENCLAVE_MYENCLAVE_TRUSTED_LIB_PACKETS_H_
#include "sgx_htons.h"
#include "flow.h"
#include <stdbool.h>
#include "../include/types.h"

#define ETH_TYPE_IP            0x0800
#define ETH_TYPE_ARP           0x0806
#define ETH_TYPE_VLAN_8021Q    0x8100
#define ETH_TYPE_VLAN          ETH_TYPE_VLAN_8021Q
#define ETH_TYPE_VLAN_8021AD   0x88a8
#define ETH_TYPE_IPV6          0x86dd
#define ETH_TYPE_LACP          0x8809
#define ETH_TYPE_RARP          0x8035
#define ETH_TYPE_MPLS          0x8847
#define ETH_TYPE_MPLS_MCAST    0x8848
#define VLAN_CFI 0x1000

#define VLAN_VID_MASK 0x0fff
#define VLAN_VID_SHIFT 0

#define VLAN_PCP_MASK 0xe000
#define VLAN_PCP_SHIFT 13

#define VLAN_CFI 0x1000
#define VLAN_CFI_SHIFT 12


#define ND_NEIGHBOR_SOLICIT		135	/* neighbor solicitation */
#define ND_NEIGHBOR_ADVERT		136	/* neighbor advertisement */

#define IPPROTO_TCP 6
#define IPPROTO_UDP 17
#define IPPROTO_ICMP 1
#define IPPROTO_ICMPV6 58

#define ETH_ADDR_LEN           6
/* TOS fields. */
#define IP_ECN_NOT_ECT 0x0
#define IP_ECN_ECT_1 0x01
#define IP_ECN_ECT_0 0x02
#define IP_ECN_CE 0x03
#define IP_ECN_MASK 0x03
#define IP_DSCP_MASK 0xfc

#define IP_VERSION 4
/* MPLS related definitions */
#define MPLS_TTL_MASK       0x000000ff
#define MPLS_TTL_SHIFT      0

#define MPLS_BOS_MASK       0x00000100
#define MPLS_BOS_SHIFT      8

#define MPLS_TC_MASK        0x00000e00
#define MPLS_TC_SHIFT       9

#define MPLS_LABEL_MASK     0xfffff000
#define MPLS_LABEL_SHIFT    12

#define MPLS_HLEN           4


static inline bool eth_type_mpls(ovs_be16 eth_type)
{
    return eth_type == htons(ETH_TYPE_MPLS) ||
        eth_type == htons(ETH_TYPE_MPLS_MCAST);
}

static inline bool dl_type_is_ip_any(ovs_be16 dl_type)
{
    return dl_type == htons(ETH_TYPE_IP)
        || dl_type == htons(ETH_TYPE_IPV6);
}

static inline bool is_ip_any(const struct flow *flow)
{
    return dl_type_is_ip_any(flow->dl_type);
}

static inline bool is_icmpv4(const struct flow *flow)
{
    return (flow->dl_type == htons(ETH_TYPE_IP)
            && flow->nw_proto == IPPROTO_ICMP);
}

static inline bool is_icmpv6(const struct flow *flow)
{
    return (flow->dl_type == htons(ETH_TYPE_IPV6)
            && flow->nw_proto == IPPROTO_ICMPV6);
}


/* Given a mpls label stack entry in network byte order
 * return mpls BoS bit  */
static inline uint8_t
mpls_lse_to_bos(ovs_be32 mpls_lse)
{
    return (mpls_lse & htonl(MPLS_BOS_MASK)) != 0;
}

/* Given a mpls label stack entry in network byte order
 * return mpls tc */
static inline uint8_t
mpls_lse_to_tc(ovs_be32 mpls_lse)
{
    return (ntohl(mpls_lse) & MPLS_TC_MASK) >> MPLS_TC_SHIFT;
}

/* Given the vlan_tci field from an 802.1Q header, in network byte order,
 * returns the priority code point (PCP) in host byte order. */
static inline int
vlan_tci_to_pcp(ovs_be16 vlan_tci)
{
    return (ntohs(vlan_tci) & VLAN_PCP_MASK) >> VLAN_PCP_SHIFT;
}

/* Given a mpls label stack entry in network byte order
 * return mpls label in host byte order */
static inline uint32_t
mpls_lse_to_label(ovs_be32 mpls_lse)
{
    return (ntohl(mpls_lse) & MPLS_LABEL_MASK) >> MPLS_LABEL_SHIFT;
}

/* Given the vlan_tci field from an 802.1Q header, in network byte order,
 * returns the VLAN ID in host byte order. */
static inline uint16_t
vlan_tci_to_vid(ovs_be16 vlan_tci)
{
    return (ntohs(vlan_tci) & VLAN_VID_MASK) >> VLAN_VID_SHIFT;
}

#endif /* ENCLAVE_MYENCLAVE_TRUSTED_LIB_PACKETS_H_ */
