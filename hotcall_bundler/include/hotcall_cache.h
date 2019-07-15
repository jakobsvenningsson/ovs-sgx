#ifndef _HOTCALL_FUNCTION_CACHE_MEMO_
#define _HOTCALL_FUNCTION_CACHE_MEMO_

#include "hotcall-list.h"
#include "hotcall-hmap.h"

struct function_cache_ctx {
    struct hcall_list lru_list;
    struct hcall_hmap cache;
    struct hcall_hmap val_cache;
    unsigned int cache_sz;
    /*char pad[64];
    struct hcall_list *tmp_lru[10];
    unsigned int tmp_lru_idx;
    unsigned int n_lru;*/
};


union cache_entry_type {
    int INT_TYPE;
    unsigned int UNSIGNED_TYPE;
    bool BOOL_TYPE;
    void *POINTER_TYPE;
};

struct cache_entry {
    union cache_entry_type type;
    struct hcall_hmap_node hmap_node;
    struct hcall_hmap_node hmap_val_node;
    struct hcall_list lru_list_node;
    char tag;
};

#define ACCESS_FIELD(STRUCT, FIELD) STRUCT.FIELD

#endif
