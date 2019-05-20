/*
 * ofproto-provider.h
 *
 *  Created on: Apr 24, 2018
 *      Author: jorge
 */

#ifndef ENCLAVE_MYENCLAVE_TRUSTED_LIB_OFPROTO_PROVIDER_H_
#define ENCLAVE_MYENCLAVE_TRUSTED_LIB_OFPROTO_PROVIDER_H_

#include "classifier.h"
#include "hmap.h"
#include "meta-flow.h"
#include "heap.h"


struct eviction_group {
    struct hmap_node id_node;   /* In oftable's "eviction_groups_by_id". */
    struct heap_node size_node; /* In oftable's "eviction_groups_by_size". */
    struct heap rules;          /* Contains "struct rule"s. */
    struct list *block_list_node;
};

/* Number of implemented OpenFlow tables. */
enum { N_TABLES = 255 };
enum { TBL_INTERNAL = N_TABLES - 1 };    /* Used for internal hidden rules. */
#define OFP_MAX_TABLE_NAME_LEN 32

/*Definition of oftable and utilities*/

/* OpenFlow table flags:
 *
 *   - "Hidden" tables are not included in OpenFlow operations that operate on
 *     "all tables".  For example, a request for flow stats on all tables will
 *     omit flows in hidden tables, table stats requests will omit the table
 *     entirely, and the switch features reply will not count the hidden table.
 *
 *     However, operations that specifically name the particular table still
 *     operate on it.  For example, flow_mods and flow stats requests on a
 *     hidden table work.
 *
 *     To avoid gaps in table IDs (which have unclear validity in OpenFlow),
 *     hidden tables must be the highest-numbered tables that a provider
 *     implements.
 *
 *   - "Read-only" tables can't be changed through OpenFlow operations.  (At
 *     the moment all flow table operations go effectively through OpenFlow, so
 *     this means that read-only tables can't be changed at all after the
 *     read-only flag is set.)
 *
 * The generic ofproto layer never sets these flags.  An ofproto provider can
 * set them if it is appropriate.
 */
enum oftable_flags {
    OFTABLE_HIDDEN = 1 << 0,   /* Hide from most OpenFlow operations. */
    OFTABLE_READONLY = 1 << 1  /* Don't allow OpenFlow to change this table. */
};

/* A flow table within a "struct ofproto". */
struct oftable {
    enum oftable_flags flags;
    struct classifier cls;      /* Contains "struct rule"s. */
    char *name;                 /* Table name exposed via OpenFlow, or NULL. */

    /* Maximum number of flows or UINT_MAX if there is no limit besides any
     * limit imposed by resource limitations. */
    unsigned int max_flows;

    /* These members determine the handling of an attempt to add a flow that
     * would cause the table to have more than 'max_flows' flows.
     *
     * If 'eviction_fields' is NULL, overflows will be rejected with an error.
     *
     * If 'eviction_fields' is nonnull (regardless of whether n_eviction_fields
     * is nonzero), an overflow will cause a flow to be removed.  The flow to
     * be removed is chosen to give fairness among groups distinguished by
     * different values for the subfields within 'groups'. */
    struct mf_subfield *eviction_fields;
    size_t n_eviction_fields;

    /* Eviction groups.
     *
     * When a flow is added that would cause the table to have more than
     * 'max_flows' flows, and 'eviction_fields' is nonnull, these groups are
     * used to decide which rule to evict: the rule is chosen from the eviction
     * group that contains the greatest number of rules.*/
    uint32_t eviction_group_id_basis;
    struct hmap eviction_groups_by_id;
    struct heap eviction_groups_by_size;
};

//Internal hmap table

struct sgx_cls_table {
	struct hmap cls_rules;
	//int n_table_rules;
};

//Internal Enclave cls_rule structure
struct sgx_cls_rule {
	//size_t p_cls_rule;
	struct cls_rule  cls_rule; //Pointer of cls_rule in untrusted memory
	struct cls_rule *o_cls_rule;  //cls rule created in trusted memory
	struct heap_node rule_evg_node; //evg_node in a rule
	struct eviction_group *evict_group; //points to a struct evict in trusted memory
	bool evictable;              /* If false, prevents eviction. */
	//struct sgx_cls_rule *node;
	struct hmap_node hmap_node;
    struct list *block_list_node;


    uint16_t hard_timeout;       /* In seconds from ->modified. */
    uint16_t idle_timeout;       /* In seconds from ->used. */
};

/*Struct SGX_table_dpif: is a struct to store in trusted memory
 * the pointer of cls_subtables
 */

struct SGX_table_dpif {
//	int table_id;
	struct cls_table *catchall_table; /* Table that wildcards all fields. */
	struct cls_table *other_table;    /* Table with any other wildcard set. */
	uint32_t basis;
};


enum nx_flow_monitor_flags {
    /* When to send updates. */
    NXFMF_INITIAL = 1 << 0,     /* Initially matching flows. */
    NXFMF_ADD = 1 << 1,         /* New matching flows as they are added. */
    NXFMF_DELETE = 1 << 2,      /* Old matching flows as they are removed. */
    NXFMF_MODIFY = 1 << 3,      /* Matching flows as they are changed. */

    /* What to include in updates. */
    NXFMF_ACTIONS = 1 << 4,     /* If set, actions are included. */
    NXFMF_OWN = 1 << 5,         /* If set, include own changes in full. */
};

struct rule {
    struct list ofproto_node;    /* Owned by ofproto base code. */
    void *ofproto;     /* The ofproto that contains this rule. */
    struct cls_rule cr;          /* In owning ofproto's classifier. */

    void *pending; /* Operation now in progress, if nonnull. */

    ovs_be64 flow_cookie;        /* Controller-issued identifier. */

    long long int created;       /* Creation time. */
    long long int modified;      /* Time of last modification. */
    long long int used;          /* Last use; time created if never used. */
    uint16_t hard_timeout;       /* In seconds from ->modified. */
    uint16_t idle_timeout;       /* In seconds from ->used. */
    uint8_t table_id;            /* Index in ofproto's 'tables' array. */
    bool send_flow_removed;      /* Send a flow removed message? */

    /* Eviction groups. */
    #ifndef SGX
    bool evictable;              /* If false, prevents eviction. */
    struct heap_node evg_node;   /* In eviction_group's "rules" heap. */
    struct eviction_group *eviction_group; /* NULL if not in any group. */
    #endif

    void *ofpacts;      /* Sequence of "struct ofpacts". */
    unsigned int ofpacts_len;    /* Size of 'ofpacts', in bytes. */

    /* Flow monitors. */
    enum nx_flow_monitor_flags monitor_flags;
    uint64_t add_seqno;         /* Sequence number when added. */
    uint64_t modify_seqno;      /* Sequence number when changed. */

    /* Optimisation for flow expiry. */
    struct list expirable;      /* In ofproto's 'expirable' list if this rule
                                 * is expirable, otherwise empty. */

    uint16_t tmp_storage_vid;
    uint8_t tmp_storage_vid_exist;
    uint16_t tmp_storage_vid_mask;
    uint8_t tmp_storage_vid_mask_exist;

    bool is_other_table;
    int table_update_taggable;
};






/* Returns the priority to use for an eviction_group that contains 'n_rules'
 * rules.  The priority contains low-order random bits to ensure that eviction
 * groups with the same number of rules are prioritized randomly. */

#endif /* ENCLAVE_MYENCLAVE_TRUSTED_LIB_OFPROTO_PROVIDER_H_ */
