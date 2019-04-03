/*
 * meta-flow.h
 *
 *  Created on: Apr 23, 2018
 *      Author: jorge
 */

#ifndef ENCLAVE_MYENCLAVE_TRUSTED_LIB_META_FLOW_H_
#define ENCLAVE_MYENCLAVE_TRUSTED_LIB_META_FLOW_H_

#include "packets.h"
#include "flow.h"



/* The comment on each of these indicates the member in "union mf_value" used
 * to represent its value. */
enum mf_field_id {
    /* Metadata. */
    MFF_TUN_ID,                 /* be64 */
    MFF_TUN_SRC,                /* be32 */
    MFF_TUN_DST,                /* be32 */
    MFF_TUN_FLAGS,              /* be16 */
    MFF_TUN_TTL,                /* u8 */
    MFF_TUN_TOS,                /* u8 */
    MFF_METADATA,               /* be64 */
    MFF_IN_PORT,                /* be16 */
    MFF_SKB_PRIORITY,           /* be32 */
    MFF_SKB_MARK,               /* be32 */

#if FLOW_N_REGS > 0
    MFF_REG0,                   /* be32 */
#endif
#if FLOW_N_REGS > 1
    MFF_REG1,                   /* be32 */
#endif
#if FLOW_N_REGS > 2
    MFF_REG2,                   /* be32 */
#endif
#if FLOW_N_REGS > 3
    MFF_REG3,                   /* be32 */
#endif
#if FLOW_N_REGS > 4
    MFF_REG4,                   /* be32 */
#endif
#if FLOW_N_REGS > 5
    MFF_REG5,                   /* be32 */
#endif
#if FLOW_N_REGS > 6
    MFF_REG6,                   /* be32 */
#endif
#if FLOW_N_REGS > 7
    MFF_REG7,                   /* be32 */
#endif

    /* L2. */
    MFF_ETH_SRC,                /* mac */
    MFF_ETH_DST,                /* mac */
    MFF_ETH_TYPE,               /* be16 */

    MFF_VLAN_TCI,               /* be16 */
    MFF_DL_VLAN,                /* be16 (OpenFlow 1.0 compatibility) */
    MFF_VLAN_VID,               /* be16 (OpenFlow 1.2 compatibility) */
    MFF_DL_VLAN_PCP,            /* u8 (OpenFlow 1.0 compatibility) */
    MFF_VLAN_PCP,               /* be16 (OpenFlow 1.2 compatibility) */

    /* L2.5 */
    MFF_MPLS_LABEL,             /* be32 */
    MFF_MPLS_TC,                /* u8 */
    MFF_MPLS_BOS,               /* u8 */

    /* L3. */
    MFF_IPV4_SRC,               /* be32 */
    MFF_IPV4_DST,               /* be32 */

    MFF_IPV6_SRC,               /* ipv6 */
    MFF_IPV6_DST,               /* ipv6 */
    MFF_IPV6_LABEL,             /* be32 */

    /* The IPv4/IPv6 DSCP field has two different views:
     *
     *   - MFF_IP_DSCP has the DSCP in bits 2-7, their bit positions in the
     *     IPv4 and IPv6 "traffic class" field, as used in OpenFlow 1.0 and 1.1
     *     flow format and in NXM's NXM_OF_IP_TOS
     *
     *   - MFF_IP_DSCP has the DSCP in bits 0-5, shifted right two bits from
     *     their positions in the IPv4 and IPv6 "traffic class" field, as used
     *     in OpenFlow 1.2+ OXM's OXM_OF_IP_DSCP. */
    MFF_IP_PROTO,               /* u8 (used for IPv4 or IPv6) */
    MFF_IP_DSCP,                /* u8 (used for IPv4 or IPv6) */
    MFF_IP_DSCP_SHIFTED,        /* u8 (used for IPv4 or IPv6) (OF1.2 compat) */
    MFF_IP_ECN,                 /* u8 (used for IPv4 or IPv6) */
    MFF_IP_TTL,                 /* u8 (used for IPv4 or IPv6) */
    MFF_IP_FRAG,                /* u8 (used for IPv4 or IPv6) */

    MFF_ARP_OP,                 /* be16 */
    MFF_ARP_SPA,                /* be32 */
    MFF_ARP_TPA,                /* be32 */
    MFF_ARP_SHA,                /* mac */
    MFF_ARP_THA,                /* mac */

    /* L4. */
    MFF_TCP_SRC,                /* be16 (used for IPv4 or IPv6) */
    MFF_TCP_DST,                /* be16 (used for IPv4 or IPv6) */

    MFF_UDP_SRC,                /* be16 (used for IPv4 or IPv6) */
    MFF_UDP_DST,                /* be16 (used for IPv4 or IPv6) */

    MFF_ICMPV4_TYPE,            /* u8 */
    MFF_ICMPV4_CODE,            /* u8 */

    MFF_ICMPV6_TYPE,            /* u8 */
    MFF_ICMPV6_CODE,            /* u8 */

    /* ICMPv6 Neighbor Discovery. */
    MFF_ND_TARGET,              /* ipv6 */
    MFF_ND_SLL,                 /* mac */
    MFF_ND_TLL,                 /* mac */

    MFF_N_IDS
};

/* Use this macro as CASE_MFF_REGS: in a switch statement to choose all of the
 * MFF_REGx cases. */
#if FLOW_N_REGS == 1
# define CASE_MFF_REGS                                          \
    case MFF_REG0
#elif FLOW_N_REGS == 2
# define CASE_MFF_REGS                                          \
    case MFF_REG0: case MFF_REG1
#elif FLOW_N_REGS == 3
# define CASE_MFF_REGS                                          \
    case MFF_REG0: case MFF_REG1: case MFF_REG2
#elif FLOW_N_REGS == 4
# define CASE_MFF_REGS                                          \
    case MFF_REG0: case MFF_REG1: case MFF_REG2: case MFF_REG3
#elif FLOW_N_REGS == 5
# define CASE_MFF_REGS                                          \
    case MFF_REG0: case MFF_REG1: case MFF_REG2: case MFF_REG3: \
    case MFF_REG4
#elif FLOW_N_REGS == 6
# define CASE_MFF_REGS                                          \
    case MFF_REG0: case MFF_REG1: case MFF_REG2: case MFF_REG3: \
    case MFF_REG4: case MFF_REG5
#elif FLOW_N_REGS == 7
# define CASE_MFF_REGS                                          \
    case MFF_REG0: case MFF_REG1: case MFF_REG2: case MFF_REG3: \
    case MFF_REG4: case MFF_REG5: case MFF_REG6
#elif FLOW_N_REGS == 8
# define CASE_MFF_REGS                                          \
    case MFF_REG0: case MFF_REG1: case MFF_REG2: case MFF_REG3: \
    case MFF_REG4: case MFF_REG5: case MFF_REG6: case MFF_REG7
#else
# error
#endif

/* Prerequisites for matching a field.
 *
 * A field may only be matched if the correct lower-level protocols are also
 * matched.  For example, the TCP port may be matched only if the Ethernet type
 * matches ETH_TYPE_IP and the IP protocol matches IPPROTO_TCP. */
enum mf_prereqs {
    MFP_NONE,

    /* L2 requirements. */
    MFP_ARP,
    MFP_VLAN_VID,
    MFP_IPV4,
    MFP_IPV6,
    MFP_IP_ANY,

    /* L2.5 requirements. */
    MFP_MPLS,

    /* L2+L3 requirements. */
    MFP_TCP,                    /* On IPv4 or IPv6. */
    MFP_UDP,                    /* On IPv4 or IPv6. */
    MFP_ICMPV4,
    MFP_ICMPV6,

    /* L2+L3+L4 requirements. */
    MFP_ND,
    MFP_ND_SOLICIT,
    MFP_ND_ADVERT
};

/* Forms of partial-field masking allowed for a field.
 *
 * Every field may be masked as a whole. */
enum mf_maskable {
    MFM_NONE,                   /* No sub-field masking. */
    MFM_FULLY,                  /* Every bit is individually maskable. */
};

/* How to format or parse a field's value. */
enum mf_string {
    /* Integer formats.
     *
     * The particular MFS_* constant sets the output format.  On input, either
     * decimal or hexadecimal (prefixed with 0x) is accepted. */
    MFS_DECIMAL,
    MFS_HEXADECIMAL,

    /* Other formats. */
    MFS_ETHERNET,
    MFS_IPV4,
    MFS_IPV6,
    MFS_OFP_PORT,               /* An OpenFlow port number or name. */
    MFS_FRAG,                   /* no, yes, first, later, not_later */
    MFS_TNL_FLAGS,              /* FLOW_TNL_F_* flags */
};

struct mf_field {
    /* Identification. */
    enum mf_field_id id;        /* MFF_*. */
    const char *name;           /* Name of this field, e.g. "eth_type". */
    const char *extra_name;     /* Alternate name, e.g. "dl_type", or NULL. */

    /* Size.
     *
     * Most fields have n_bytes * 8 == n_bits.  There are a few exceptions:
     *
     *     - "dl_vlan" is 2 bytes but only 12 bits.
     *     - "dl_vlan_pcp" is 1 byte but only 3 bits.
     *     - "is_frag" is 1 byte but only 2 bits.
     *     - "ipv6_label" is 4 bytes but only 20 bits.
     *     - "mpls_label" is 4 bytes but only 20 bits.
     *     - "mpls_tc"    is 1 byte but only 3 bits.
     *     - "mpls_bos"   is 1 byte but only 1 bit.
     */
    unsigned int n_bytes;       /* Width of the field in bytes. */
    unsigned int n_bits;        /* Number of significant bits in field. */

    /* Properties. */
    enum mf_maskable maskable;
    enum mf_string string;
    enum mf_prereqs prereqs;
    bool writable;              /* May be written by actions? */

    /* NXM and OXM properties.
     *
     * There are the following possibilities for these members for a given
     * mf_field:
     *
     *   - Neither NXM nor OXM defines such a field: these members will all be
     *     zero or NULL.
     *
     *   - NXM and OXM both define such a field: nxm_header and oxm_header will
     *     both be nonzero and different, similarly for nxm_name and oxm_name.
     *
     *   - Only NXM or only OXM defines such a field: nxm_header and oxm_header
     *     will both have the same value (either an OXM_* or NXM_* value) and
     *     similarly for nxm_name and oxm_name.
     *
     * Thus, 'nxm_header' is the appropriate header to use when outputting an
     * NXM formatted match, since it will be an NXM_* constant when possible
     * for compatibility with OpenFlow implementations that expect that, with
     * OXM_* constants used for fields that OXM adds.  Conversely, 'oxm_header'
     * is the header to use when outputting an OXM formatted match. */
    uint32_t nxm_header;        /* An NXM_* (or OXM_*) constant. */
    const char *nxm_name;       /* The nxm_header constant's name. */
    uint32_t oxm_header;        /* An OXM_* (or NXM_*) constant. */
    const char *oxm_name;	    /* The oxm_header constant's name */
};

/* Part of a field. */
struct mf_subfield {
    const struct mf_field *field;
    unsigned int ofs;           /* Bit offset. */
    unsigned int n_bits;        /* Number of bits. */
};
/* The representation of a field's value. */
union mf_value {
    uint8_t u8;
    ovs_be16 be16;
    ovs_be32 be32;
    ovs_be64 be64;
    uint8_t mac[ETH_ADDR_LEN];
    struct in6_addr ipv6;
};

bool
mf_are_prereqs_ok(const struct mf_field *mf, const struct flow *flow);






#endif /* ENCLAVE_MYENCLAVE_TRUSTED_LIB_META_FLOW_H_ */
