#ifndef _H_CACHE_UNTRUSTED_H
#define _H_CACHE_UNTRUSTED_H

#include "common.h"
#include <stdint.h>
#include "enclave_u.h"


#define MAX_CACHE_SIZE 5

size_t
flow_map_cache_calculate_hash(const struct flow *flow, const struct flow_wildcards *wc, int bridge_id, int table_id);
cls_cache_entry *
flow_map_cache_get_entry(struct function_cache_ctx *f_ctx, const struct flow *flow, const struct flow_wildcards *wc, int bridge_id, int table_id, struct cache_entry **ce);
void
flow_map_cache_init(flow_map_cache *flow_cache);
void
flow_map_cache_insert(struct function_cache_ctx *f_ctx, const struct flow *flow, const struct flow_wildcards *wc, struct cls_rule *ut_cr, int bridge_id, int table_id);
static inline uint32_t
ut_cr_addr_hash(struct cls_rule *ut_cr) {
    return hash_bytes(ut_cr, 8, 0);
}

static inline uint32_t
cache_entry_hash(cls_cache_entry *ce, uint32_t basis) {
    return hash_int(ce->hmap_node.hash + (size_t) ce->cr, basis);
}

#endif
