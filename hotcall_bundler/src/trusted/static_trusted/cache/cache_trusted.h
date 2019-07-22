#ifndef _H_LIB_HOTCALL_CACHE_TRUSTED_
#define _H_LIB_HOTCALL_CACHE_TRUSTED_

#include <stdint.h>
#include "hotcall.h"
#include "hotcall_cache.h"
#include "hotcall-hmap.h"


void static inline
memoize_value(struct memoize *mem, struct hotcall_function_config *config, void *val, void *writeback_hash) {
    if(config->memoize.return_type == 'p') {
        if(*(void **) val == NULL) return;
    }

    struct function_cache_ctx *f_ctx = mem->functions[config->function_id];
    struct hcall_list *front = hcall_list_pop_front(&f_ctx->lru_list);
    hcall_list_push_back(&f_ctx->lru_list, front);
    struct cache_entry *ce = CONTAINER_OF(front, struct cache_entry, lru_list_node);
    if(hcall_hmap_contains(&f_ctx->cache, &ce->hmap_node)) {
        hcall_hmap_remove(&f_ctx->cache, &ce->hmap_node);
    }
    if(hcall_hmap_contains(&f_ctx->val_cache, &ce->hmap_val_node)) {
        hcall_hmap_remove(&f_ctx->val_cache, &ce->hmap_val_node);
    }

    uint32_t hash = config->memoize.manual_update ? *(uint32_t *) writeback_hash : config->memoize.hash;
    hcall_hmap_insert_fast(&f_ctx->cache, &ce->hmap_node, hash);

    switch(config->memoize.return_type) {
        case 'd':
            ce->type.INT_TYPE = *(int *) val;
            hcall_hmap_insert_fast(&f_ctx->val_cache, &ce->hmap_val_node, ce->type.INT_TYPE);
            break;
        case 'u':
            ce->type.UNSIGNED_TYPE = *(unsigned int *) val;
            hcall_hmap_insert_fast(&f_ctx->val_cache, &ce->hmap_val_node, ce->type.UNSIGNED_TYPE);
            break;
        case 'b':
            ce->type.BOOL_TYPE = *(bool *) val;
            hcall_hmap_insert_fast(&f_ctx->val_cache, &ce->hmap_val_node, ce->type.BOOL_TYPE);
            break;
        case 'p':
            ce->type.POINTER_TYPE = *(void **) val;
            hcall_hmap_insert_fast(&f_ctx->val_cache, &ce->hmap_val_node, (uintptr_t) ce->type.POINTER_TYPE);
            break;
        case ui16:
            ce->type.UNSIGNED_TYPE_16 = *(uint16_t *) val;
            hcall_hmap_insert_fast(&f_ctx->val_cache, &ce->hmap_val_node, ce->type.UNSIGNED_TYPE_16);
            break;
        default: SWITCH_DEFAULT_REACHED
    }
}

void static inline
invalidate_cache(struct memoize *mem, struct memoize_cache *cache) {
    struct function_cache_ctx *f_ctx;
    f_ctx = mem->functions[cache->id];
    hcall_hmap_clear(&f_ctx->val_cache);
    hcall_hmap_clear(&f_ctx->cache);
}

void static inline
invalidate_cache_line(struct memoize *mem, struct memoize_cache *cache, void *val) {
    struct function_cache_ctx *f_ctx = NULL;
    f_ctx = mem->functions[cache->id];
    if(!f_ctx) return;
    switch(cache->type) {
        case VALUE:
        {
            struct hcall_hmap_node *hmap_node;
            struct cache_entry *ce;
            while((hmap_node = hcall_hmap_first_with_hash(&f_ctx->val_cache, cache->invalidate_element.hash)) != NULL) {
                ce = CONTAINER_OF(hmap_node, struct cache_entry, hmap_node);
                hcall_hmap_remove(&f_ctx->val_cache, &ce->hmap_val_node);
                hcall_hmap_remove(&f_ctx->cache, &ce->hmap_node);
            }
            break;
        }
        case HASH:
        {
            struct hcall_hmap_node *hmap_node;
            struct cache_entry *ce;
            while((hmap_node = hcall_hmap_first_with_hash(&f_ctx->cache, cache->invalidate_element.hash)) != NULL) {
                ce = CONTAINER_OF(hmap_node, struct cache_entry, hmap_node);
                hcall_hmap_remove(&f_ctx->val_cache, &ce->hmap_val_node);
                hcall_hmap_remove(&f_ctx->cache, &ce->hmap_node);
            }
            break;
        }
        case RETURN_VALUE:
        {
            uint32_t val_hash;
            switch(cache->invalidate_element.fmt) {
                case 'd':
                    val_hash = *(int *) val;
                    break;
                case 'u':
                    val_hash = *(unsigned int *) val;
                    break;
                case 'p':
                    if(*(void **) val == NULL) return;
                    val_hash = *(bool *) val;
                    break;
                default: SWITCH_DEFAULT_REACHED
            }

            struct hcall_hmap_node *hmap_node;
            struct cache_entry *ce;
            while((hmap_node = hcall_hmap_first_with_hash(&f_ctx->val_cache, val_hash)) != NULL) {
                ce = CONTAINER_OF(hmap_node, struct cache_entry, hmap_val_node);
                hcall_hmap_remove(&f_ctx->val_cache, &ce->hmap_val_node);
                hcall_hmap_remove(&f_ctx->cache, &ce->hmap_node);
            }
            break;
        }
        case CLEAR_CACHE:
            invalidate_cache(mem, cache);
            break;
        default: SWITCH_DEFAULT_REACHED
    }
}

#endif
