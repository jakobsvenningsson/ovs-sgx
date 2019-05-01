#ifndef _H_CACHE_UNTRUSTED_H
#define _H_CACHE_UNTRUSTED_H

#include "common.h"
#include <stdint.h>
#include "enclave_u.h"

size_t flow_map_cache_calculate_hash(const struct flow *flow, const struct flow_wildcards *wc, int bridge_id, int table_id);
cls_cache_entry * flow_map_cache_get_entry(flow_map_cache *flow_cache, const struct flow *flow, const struct flow_wildcards *wc, int bridge_id, int table_id);

#endif
