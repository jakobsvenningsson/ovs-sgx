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



size_t
allocate_page(shared_memory *shared_memory, size_t page_sz) {
    int page_n = -1;
    for(int i = 0; i < shared_memory->cap; ++i) {
        if(!shared_memory->allocated[i]) {
            page_n = i;
            break;
        }
    }
    if(page_n == -1) {
        return -1;
    }

    printf("Allocating page %d of size %zu\n", page_n, (page_sz ? page_sz : shared_memory->default_page_sz));


    shared_memory->allocated[page_n] = 1;
    size_t _page_size = page_sz ? page_sz : shared_memory->default_page_sz;
    shared_memory->pages[page_n] = malloc(_page_size);
    shared_memory->page_sz[page_n] = _page_size;

    return page_n;
}

void
initialize_shared_memory(shared_memory *shared_memory) {
    shared_memory->cap = MAX_N_PAGES;
    shared_memory->allocated = calloc(shared_memory->cap, sizeof(uint8_t));
    shared_memory->page_sz = calloc(shared_memory->cap, sizeof(size_t));
    shared_memory->deallocate_page = calloc(shared_memory->cap, sizeof(uint8_t));
    shared_memory->page_type = calloc(shared_memory->cap, sizeof(uint8_t));

    shared_memory->default_page_sz = DEFAULT_PAGE_SIZE;
    shared_memory->pages = (uint8_t **) malloc(shared_memory->cap * sizeof(uint8_t *));

    int res;
    for(size_t i = 0; i < MAX_N_PAGES/2; ++i) {
        res = allocate_page(shared_memory, 0);
        if(res == -1) {
            printf("Init: Failed to allocate page.\n");
            exit(-1);
        }
    }
}

void
initialize_enclave_cache(flow_map_cache *flow_cache) {

    flow_cache->cap = 100;
    hmap_init(&flow_cache->entries);
    hmap_init(&flow_cache->ut_crs);

    initialize_shared_memory(&flow_cache->shared_memory);

    list_init(&flow_cache->lru_list);
    cls_cache_entry *cache_entry;
    for(size_t i = 0; i < flow_cache->cap; ++i) {
        cache_entry = malloc(sizeof(cls_cache_entry));
        list_insert(&flow_cache->lru_list, &cache_entry->list_node);
    }
}

void
deallocate_marked_pages(shared_memory *shared_memory) {
    for(size_t i = 0; i < shared_memory->cap; ++i) {
        if(shared_memory->deallocate_page[i]) {
            shared_memory->page_sz[i] = 0;
            shared_memory->deallocate_page[i] = 0;
            shared_memory->allocated[i] = 0;
            shared_memory->page_type[i] = 0;
            free(shared_memory->pages[i]);
        }
    }
}
