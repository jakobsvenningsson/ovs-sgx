#include "cache-untrusted.h"

size_t flow_map_cache_calculate_hash(const struct flow *flow, const struct flow_wildcards *wc, int bridge_id, int table_id) {
    size_t hash;
    if(wc) {
        struct minimask m;
        minimask_init(&m, wc);
        hash = flow_hash_in_minimask(flow, &m, 0);
    } else {
        hash = flow_hash(flow, 0);
    }
    hash += bridge_id + table_id;
    return hash;
}

cls_cache_entry * flow_map_cache_get_entry(flow_map_cache *flow_cache, const struct flow *flow, const struct flow_wildcards *wc, int bridge_id, int table_id) {
    size_t hash;
    hash = flow_map_cache_calculate_hash(flow, wc, bridge_id, table_id);
    cls_cache_entry *cache_entry;
    HMAP_FOR_EACH_WITH_HASH(cache_entry, hmap_node, hash, &flow_cache->entries) {
        list_remove(&cache_entry->list_node);
        list_push_back(&flow_cache->lru_list, &cache_entry->list_node);
        return cache_entry;
    }
    return NULL;
}
