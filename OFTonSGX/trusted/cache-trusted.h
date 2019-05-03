#ifndef _H_CACHE_TRUSTED_
#define _H_CACHE_TRUSTED_

#include "common.h"

#include "ofproto-provider.h"
#include "hmap.h"
#include "flow.h"

#define FLOW_CACHE_UPDATE_TYPE_INSERT 0
#define FLOW_CACHE_UPDATE_TYPE_DELETE 1

bool
flow_map_cache_is_valid(flow_map_cache *flow_cache);
size_t
flow_map_cache_calculate_hash(const struct flow *flow, const struct flow_wildcards *wc, int bridge_id, int table_id);
void
flow_map_cache_remove_with_hash(flow_map_cache *flow_cache, const struct cls_rule *t_cr, int bridge_id, int table_id);
void
flow_map_cache_remove_ut_cr(flow_map_cache *flow_cache, struct cls_rule *ut_cr);
void
flow_map_cache_insert(flow_map_cache *flow_cache, const struct flow *flow, const struct flow_wildcards *wc, struct cls_rule *ut_cr, int bridge_id, int table_id);
void
flow_map_cache_insert_rule(flow_map_cache *flow_cache, struct cls_rule *t_cr, struct cls_rule *ut_cr, int bridge_id, int table_id);
void
flow_map_cache_flush(flow_map_cache *flow_cache);

static inline uint32_t
ut_cr_addr_hash(struct cls_rule *ut_cr) {
    return hash_bytes(ut_cr, 8, 0);
}

static inline uint32_t
cache_entry_hash(cls_cache_entry *ce, uint32_t basis) {
    return hash_int(ce->hmap_node.hash + (size_t) ce->cr, basis);
}

#endif
