#ifndef _H_CACHE_TRUSTED_
#define _H_CACHE_TRUSTED_

#include "common.h"

#include "ofproto-provider.h"
#include "hmap.h"
#include "flow.h"

#define FLOW_CACHE_UPDATE_TYPE_INSERT 0
#define FLOW_CACHE_UPDATE_TYPE_DELETE 1


void
mark_page_for_deallocation(shared_memory *shared_memory, size_t exclude_page, uint8_t page_type);
size_t
shared_memory_get_page(shared_memory *shared_memory, size_t requested_size, uint8_t page_type);
void
shared_memory_free_page(shared_memory *shared_memory, size_t exclude_page, uint8_t page_type);


bool
flow_map_cach_is_valid(struct hmap *flow_cache);
size_t
flow_map_cache_hash(struct hmap *lru_cache);
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

#endif
