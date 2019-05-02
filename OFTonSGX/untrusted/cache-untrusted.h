#ifndef _H_CACHE_UNTRUSTED_H
#define _H_CACHE_UNTRUSTED_H

#include "common.h"
#include <stdint.h>
#include "enclave_u.h"

#define DEFAULT_PAGE_SIZE 100000
#define MAX_N_PAGES 100

size_t flow_map_cache_calculate_hash(const struct flow *flow, const struct flow_wildcards *wc, int bridge_id, int table_id);
cls_cache_entry * flow_map_cache_get_entry(flow_map_cache *flow_cache, const struct flow *flow, const struct flow_wildcards *wc, int bridge_id, int table_id);

void initialize_shared_memory(shared_memory *shared_memory);
void initialize_enclave_cache(flow_map_cache *flow_cache);

size_t allocate_page(shared_memory *shared_memory, size_t page_sz);
void deallocate_marked_pages(shared_memory *shared_memory);

#endif
