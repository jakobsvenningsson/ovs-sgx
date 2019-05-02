#include "cache-trusted.h"
#include "hmap.h"
#include "flow.h"
//#include "hash.h"


void
mark_page_for_deallocation(shared_memory *shared_memory, size_t exclude_page, uint8_t page_type) {
    for(size_t i = 0; i < shared_memory->cap; ++i) {
        if(shared_memory->allocated[i] && i != exclude_page && shared_memory->page_type[i] == page_type) {
            printf("marking page %zu for deallocation.\n", i);
            shared_memory->deallocate_page[i] = 1;
        }
    }
}

size_t
shared_memory_get_page(shared_memory *shared_memory, size_t requested_size, uint8_t page_type) {
    size_t page_n = -1;
    for(size_t i = 0; i < shared_memory->cap; ++i) {
        if(shared_memory->allocated[i] &&
            requested_size <= shared_memory->page_sz[i] &&
            shared_memory->page_type[i] == PAGE_TYPE_FREE) {
            page_n = i;
            break;
        }
    }

    if(page_n == -1) {
        ocall_allocate_page(requested_size, shared_memory, &page_n);
        if(page_n == -1) {
            printf("Failed to allocate page...\n");
        }
    }
    shared_memory->page_type[page_n] = page_type;

    return page_n;
}

uint32_t ut_cr_addr_hash(struct cls_rule *ut_cr) {
    return hash_bytes(ut_cr, 8, 0);
}

void flow_map_cache_remove_with_hash(struct hmap *lru_cache, const struct cls_rule *t_cr, int bridge_id, int table_id) {
    struct flow flow;
    miniflow_expand(&t_cr->match.flow, &flow);
    struct flow_wildcards wc;
    minimask_expand(&t_cr->match.mask, &wc);

    size_t hash;
    hash = flow_map_cache_calculate_hash(&flow, &wc, bridge_id, table_id);
    cls_cache_entry *cache_entry;
    HMAP_FOR_EACH_WITH_HASH(cache_entry, hmap_node, hash, lru_cache) {
        hmap_remove(lru_cache, &cache_entry->hmap_node);
        break;
    }
}

void flow_map_cache_remove_ut_cr(flow_map_cache *flow_cache, struct cls_rule *ut_cr) {
    size_t hash = ut_cr_addr_hash(ut_cr);
    cls_cache_entry *cache_entry;
    /*HMAP_FOR_EACH_WITH_HASH(cache_entry, hmap_node_ut_crs, hash, &flow_cache->ut_crs) {
        hmap_remove(&flow_cache->entries, &cache_entry->hmap_node);
        //hmap_remove(&flow_cache->ut_crs, &cache_entry->hmap_node_ut_crs);
        break;
    }*/
    HMAP_FOR_EACH(cache_entry, hmap_node, &flow_cache->entries) {
        if(cache_entry->cr == ut_cr) {
            hmap_remove(&flow_cache->entries, &cache_entry->hmap_node);
            break;
        }
    }
}

void flow_map_cache_insert(flow_map_cache *flow_cache, const struct flow *flow, const struct flow_wildcards *wc, struct cls_rule *ut_cr, int bridge_id, int table_id) {
    size_t hash;
    hash = flow_map_cache_calculate_hash(flow, wc, bridge_id, table_id);

    cls_cache_entry *lru_entry;
    LIST_FOR_EACH(lru_entry, list_node, &flow_cache->lru_list) {
        break;
    }

    lru_entry->cr = ut_cr;
    list_remove(&lru_entry->list_node);
    list_push_back(&flow_cache->lru_list, &lru_entry->list_node);

    hmap_insert(&flow_cache->ut_crs, &lru_entry->hmap_node_ut_crs,  ut_cr_addr_hash(ut_cr), &flow_cache->shared_memory, PAGE_TYPE_UT_CRS);
    hmap_insert(&flow_cache->entries, &lru_entry->hmap_node, hash, &flow_cache->shared_memory, PAGE_TYPE_CACHE);
}
void flow_map_cache_insert_rule(flow_map_cache *flow_cache, struct cls_rule *t_cr, struct cls_rule *ut_cr, int bridge_id, int table_id) {
    struct flow flow;
    miniflow_expand(&t_cr->match.flow, &flow);
    struct flow_wildcards wc;
    minimask_expand(&t_cr->match.mask, &wc);
    flow_map_cache_insert(flow_cache, &flow, &wc, ut_cr, bridge_id, table_id);
}

void flow_map_cache_flush(flow_map_cache *flow_cache) {
    cls_cache_entry *cache_entry, *next;
    HMAP_FOR_EACH_SAFE(cache_entry, next, hmap_node, &flow_cache->entries) {
        hmap_remove(&flow_cache->entries, &cache_entry->hmap_node);
    }
    HMAP_FOR_EACH_SAFE(cache_entry, next, hmap_node_ut_crs, &flow_cache->ut_crs) {
        hmap_remove(&flow_cache->ut_crs, &cache_entry->hmap_node_ut_crs);
    }
}

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
