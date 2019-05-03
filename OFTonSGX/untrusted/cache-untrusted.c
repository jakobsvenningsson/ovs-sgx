#include "cache-untrusted.h"
#include "shared-memory-untrusted.h"
#include "spinlock.h"

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

cls_cache_entry *
flow_map_cache_get_entry(flow_map_cache *flow_cache, const struct flow *flow, const struct flow_wildcards *wc, int bridge_id, int table_id, cls_cache_entry **ce) {
    size_t hash;
    hash = flow_map_cache_calculate_hash(flow, wc, bridge_id, table_id);
    *ce = NULL;
    sgx_spin_lock(&flow_cache->shared_memory.spinlock);
    cls_cache_entry *cache_entry;
    HMAP_FOR_EACH_WITH_HASH(cache_entry, hmap_node, hash, &flow_cache->entries) {
        list_remove(&cache_entry->list_node);
        list_push_back(&flow_cache->lru_list, &cache_entry->list_node);
        *ce = cache_entry;
        break;
    }
    sgx_spin_unlock(&flow_cache->shared_memory.spinlock);
}

void
flow_map_cache_init(flow_map_cache *flow_cache) {

    flow_cache->cap = MAX_CACHE_SIZE;
    shared_memory_init(&flow_cache->shared_memory);



    /*struct hmap *hmap_cache = (struct hmap *) flow_cache->shared_memory.pages[0]->mem;
    flow_cache->shared_memory.pages[0]->status = PAGE_STATUS_CACHE;

    hmap_init(hmap_cache, &flow_cache->shared_memory);

    flow_cache->entries = *hmap_cache;

    printf("Allocating entries on addr %p\n", hmap_cache);

    struct hmap *hmap_ut = (struct hmap *) flow_cache->shared_memory.pages[1]->mem;
    flow_cache->shared_memory.pages[1]->status = PAGE_STATUS_UT_CRS;

    hmap_init(hmap_ut, &flow_cache->shared_memory);

    flow_cache->ut_crs = *hmap_ut;

    printf("Allocating ut_crs on addr %p\n", hmap_ut);*/



    hmap_init(&flow_cache->entries, &flow_cache->shared_memory);
    hmap_init(&flow_cache->ut_crs, &flow_cache->shared_memory);


    list_init(&flow_cache->lru_list);
    cls_cache_entry *cache_entry;
    for(size_t i = 0; i < flow_cache->cap; ++i) {
        cache_entry = malloc(sizeof(cls_cache_entry));
        cache_entry->cr = NULL;
        cache_entry->nr = i;
        list_insert(&flow_cache->lru_list, &cache_entry->list_node);
    }
}
