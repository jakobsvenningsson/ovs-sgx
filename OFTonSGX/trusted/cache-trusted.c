#include "cache-trusted.h"
#include "hmap.h"
#include "flow.h"
#include <stddef.h>
#include <assert.h>

// Hash value of all entries in cache, the hash is used by the enclave the validate the integrity of the cache.
static unsigned long flow_cache_hash_checksum = 0;

void
remove_cache_entry(flow_map_cache *flow_cache, cls_cache_entry *cache_entry, bool update_lru_list);
void
insert_cache_entry(flow_map_cache *flow_cache, cls_cache_entry *cache_entry, size_t hash);

unsigned long
flow_map_cache_hash(flow_map_cache *flow_cache) {
    unsigned long hash = 0;
    cls_cache_entry *cache_entry;
    sgx_spin_lock(&flow_cache->shared_memory.spinlock);
    HMAP_FOR_EACH(cache_entry, hmap_node, &flow_cache->entries) {
        //printf("Addr of hmap_node: %p, hash: %zu.\n", &cache_entry->hmap_node, cache_entry->hmap_node.hash);
        hash += (unsigned long) cache_entry_hash(cache_entry, 0);
    }
    sgx_spin_unlock(&flow_cache->shared_memory.spinlock);
    return hash;
}

bool
flow_map_cache_is_valid(flow_map_cache *flow_cache) {
    return flow_map_cache_hash(flow_cache) == flow_cache_hash_checksum;
}

size_t
flow_map_cache_update_hash(cls_cache_entry *cache_entry, int update_type) {
    uint32_t ce_hash = cache_entry_hash(cache_entry, 0);
    switch(update_type) {
        case FLOW_CACHE_UPDATE_TYPE_INSERT:
            return flow_cache_hash_checksum + ce_hash;
        case FLOW_CACHE_UPDATE_TYPE_DELETE:
            return flow_cache_hash_checksum - ce_hash;
        default:
            assert(false);
            return 0;
    }
}

/*void
flow_map_cache_remove_with_hash(flow_map_cache *flow_cache, const struct cls_rule *t_cr, int bridge_id, int table_id) {
    struct flow flow;
    miniflow_expand(&t_cr->match.flow, &flow);
    struct flow_wildcards wc;
    minimask_expand(&t_cr->match.mask, &wc);

    size_t hash;
    hash = flow_map_cache_calculate_hash(&flow, &wc, bridge_id, table_id);
    cls_cache_entry *cache_entry;
    HMAP_FOR_EACH_WITH_HASH(cache_entry, hmap_node, hash, &flow_cache->entries) {
        sgx_spin_lock(&flow_cache->shared_memory.spinlock);
        flow_cache_hash_checksum = flow_map_cache_update_hash(cache_entry, FLOW_CACHE_UPDATE_TYPE_DELETE);
        hmap_remove(&flow_cache->entries, &cache_entry->hmap_node);
        //hmap_remove(&flow_cache->ut_crs, &cache_entry->hmap_node_ut_crs);
        cache_entry->cr = NULL;
        list_remove(&cache_entry->list_node);
        list_insert(&flow_cache->lru_list, &cache_entry->list_node);
        sgx_spin_unlock(&flow_cache->shared_memory.spinlock);
        //flow_cache_hash_checksum = flow_map_cache_hash(&flow_cache->entries);

        break;
    }
}*/




void
flow_map_cache_remove_ut_cr(flow_map_cache *flow_cache, struct cls_rule *ut_cr) {
    size_t hash = ut_cr_addr_hash(ut_cr);
    cls_cache_entry *cache_entry, *next;
    sgx_spin_lock(&flow_cache->shared_memory.spinlock);
    HMAP_FOR_EACH_WITH_HASH(cache_entry, hmap_node_ut_crs, hash, &flow_cache->ut_crs) {
        if(cache_entry->cr != ut_cr) {
            continue;
        }
        flow_cache_hash_checksum = flow_map_cache_update_hash(cache_entry, FLOW_CACHE_UPDATE_TYPE_DELETE);
        remove_cache_entry(flow_cache, cache_entry, true);
        break;
    }
    sgx_spin_unlock(&flow_cache->shared_memory.spinlock);
}

void
flow_map_cache_insert(flow_map_cache *flow_cache, const struct flow *flow, const struct flow_wildcards *wc, struct cls_rule *ut_cr, int bridge_id, int table_id) {
    // Get a availible cache entry, evict if neccesary.
    sgx_spin_lock(&flow_cache->shared_memory.spinlock);
    cls_cache_entry *cache_entry;
    LIST_FOR_EACH(cache_entry, list_node, &flow_cache->lru_list) {
        if(cache_entry->cr) {
            flow_cache_hash_checksum = flow_map_cache_update_hash(cache_entry, FLOW_CACHE_UPDATE_TYPE_DELETE);
            remove_cache_entry(flow_cache, cache_entry, false);
        }
        break;
    }
    sgx_spin_unlock(&flow_cache->shared_memory.spinlock);

    cache_entry->cr = ut_cr;

    size_t hash;
    hash = flow_map_cache_calculate_hash(flow, wc, bridge_id, table_id);
    insert_cache_entry(flow_cache, cache_entry, hash);
}

void
flow_map_cache_insert_rule(flow_map_cache *flow_cache, struct cls_rule *t_cr, struct cls_rule *ut_cr, int bridge_id, int table_id) {
    struct flow flow;
    miniflow_expand(&t_cr->match.flow, &flow);
    struct flow_wildcards wc;
    minimask_expand(&t_cr->match.mask, &wc);
    flow_map_cache_insert(flow_cache, &flow, &wc, ut_cr, bridge_id, table_id);
}

void
flow_map_cache_flush(flow_map_cache *flow_cache) {
    cls_cache_entry *cache_entry, *next;
    sgx_spin_lock(&flow_cache->shared_memory.spinlock);
    HMAP_FOR_EACH_SAFE(cache_entry, next, hmap_node, &flow_cache->entries) {
        remove_cache_entry(flow_cache, cache_entry, false);
    }
    sgx_spin_unlock(&flow_cache->shared_memory.spinlock);
    flow_cache_hash_checksum = 0;
}

size_t
flow_map_cache_calculate_hash(const struct flow *flow, const struct flow_wildcards *wc, int bridge_id, int table_id) {
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


// Helper functions

void
remove_cache_entry(flow_map_cache *flow_cache, cls_cache_entry *cache_entry, bool update_lru_list) {
    hmap_remove(&flow_cache->entries, &cache_entry->hmap_node);
    hmap_remove(&flow_cache->ut_crs, &cache_entry->hmap_node_ut_crs);

    // Put cache entry in the front of lru list
    if(update_lru_list) {
        list_remove(&cache_entry->list_node);
        list_insert(&flow_cache->lru_list, &cache_entry->list_node);
    }
    cache_entry->cr = NULL;
}


void
insert_cache_entry(flow_map_cache *flow_cache, cls_cache_entry *cache_entry, size_t hash) {
    sgx_spin_lock(&flow_cache->shared_memory.spinlock);

    hmap_insert(&flow_cache->entries, &cache_entry->hmap_node, hash, &flow_cache->shared_memory, PAGE_STATUS_ALLOCATED);
    hmap_insert(&flow_cache->ut_crs, &cache_entry->hmap_node_ut_crs, hash, &flow_cache->shared_memory, PAGE_STATUS_ALLOCATED);

    // Put newly inserted cache entry in the back of the eviction queue
    list_remove(&cache_entry->list_node);
    list_push_back(&flow_cache->lru_list, &cache_entry->list_node);

    sgx_spin_unlock(&flow_cache->shared_memory.spinlock);

    flow_cache_hash_checksum = flow_map_cache_update_hash(cache_entry, FLOW_CACHE_UPDATE_TYPE_INSERT);
}
