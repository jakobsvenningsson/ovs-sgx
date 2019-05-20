#ifndef _SGX_COMMON_H
#define _SGX_COMMON_H

#include <sgx_spinlock.h>
#include <stdbool.h>
#include <stdarg.h>
#include <sgx_thread.h>

#define hotcall_ecall_test 101
#define hotcall_ecall_async_test 100
#define hotcall_ecall_delete_flows_v2 99



#define hotcall_ecall_myenclave_sample 0
#define hotcall_ecall_ofproto_init_tables 1
#define hotcall_ecall_oftable_set_readonly 2
#define hotcall_ecall_oftable_is_readonly 3
#define hotcall_ecall_cls_rule_init 4
#define hotcall_ecall_cr_rule_overlaps 5
#define hotcall_ecall_cls_rule_destroy 6
#define hotcall_ecall_cls_rule_hash 7
#define hotcall_ecall_cls_rule_equal 8
#define hotcall_ecall_oftable_classifier_replace 9
#define hotcall_ecall_oftable_get_flags 10
#define hotcall_ecall_oftable_cls_count 11
#define hotcall_ecall_is_eviction_fields_enabled 12
#define hotcall_ecall_evg_group_resize 13
#define hotcall_ecall_evg_add_rule 14
#define hotcall_ecall_evg_remove_rule 15
#define hotcall_ecall_cls_remove 16
#define hotcall_ecall_choose_rule_to_evict 17
#define hotcall_ecall_oftable_mflows 18
#define hotcall_ecall_choose_rule_to_evict_p 19
#define hotcall_ecall_minimatch_expand 20
#define hotcall_ecall_cr_priority 21
#define hotcall_ecall_oftable_cls_find_match_exactly 22
#define hotcall_ecall_collect_rules_loose_c 23
#define hotcall_ecall_collect_rules_loose_r 24
#define hotcall_ecall_collect_rules_strict_c 25
#define hotcall_ecall_collect_rules_strict_r 26
#define hotcall_ecall_oftable_enable_eviction 27
#define hotcall_ecall_oftable_disable_eviction 28
#define hotcall_ecall_oftable_enable_eviction_c 29
#define hotcall_ecall_oftable_enable_eviction_r 30
#define hotcall_ecall_oftable_mflows_set 31
#define hotcall_ecall_ofproto_destroy 32
#define hotcall_ecall_total_rules 33
#define hotcall_ecall_oftable_name 34
#define hotcall_ecall_collect_ofmonitor_util_c 35
#define hotcall_ecall_collect_ofmonitor_util_r 36
#define hotcall_ecall_cls_rule_is_loose_match 37
#define hotcall_ecall_flush_c 38
#define hotcall_ecall_flush_r 39
#define hotcall_ecall_flow_stats_c 40
#define hotcall_ecall_flow_stats_r 41
#define hotcall_ecall_oftable_cls_lookup 42
#define hotcall_ecall_cls_rule_priority 43
#define hotcall_ecall_dpif_destroy_c 44
#define hotcall_ecall_dpif_destroy_r 45
#define hotcall_ecall_cls_rule_format 46
#define hotcall_ecall_miniflow_expand 47
#define hotcall_ecall_rule_calculate_tag 48
//#define hotcall_ecall_sgx_table_dpif 49
#define hotcall_ecall_oftable_update_taggable 50
#define hotcall_ecall_oftable_is_other_table 51
#define hotcall_ecall_rule_calculate_tag_s 52
#define hotcall_ecall_oftable_hidden_check 53
#define hotcall_ecall_oftable_set_name 54
#define hotcall_ecall_minimask_get_vid_mask 55
#define hotcall_ecall_miniflow_get_vid 56
#define hotcall_ecall_ofproto_get_vlan_c 57
#define hotcall_ecall_ofproto_get_vlan_r 58

#define hotcall_ecall_oftable_get_cls_rules 59
#define hotcall_ecall_get_cls_rules_and_enable_eviction 60
#define hotcall_ecall_eviction_group_add_rules 61
#define hotcall_ecall_ofproto_get_vlan_usage 62
#define hotcall_ecall_ofproto_flush 63
#define hotcall_ecall_ofproto_evict 64
#define hotcall_ecall_add_flow 65
#define hotcall_ecall_collect_rules_loose 66
#define hotcall_ecall_collect_rules_strict 67
#define hotcall_ecall_delete_flows 68
#define hotcall_ecall_oftable_configure 69
#define hotcall_ecall_need_to_evict 70
#define hotcall_ecall_collect_rules_loose_stats_request 71
#define hotcall_ecall_ofproto_rule_send_removed 72
#define hotcall_ecall_oftable_remove_rules 73
#define hotcall_ecall_ofproto_evict_get_rest 74

#define hotcall_ecall_cls_rules_format 75
#define hotcall_ecall_minimatch_expand_and_get_priority 76
#define hotcall_ecall_miniflow_expand_and_tag 77

#define hotcall_ecall_set_evictable 78
#define hotcall_ecall_is_evictable 79
#define hotcall_ecall_restore_evictable 80
#define hotcall_ecall_backup_evictable 81
#define hotcall_ecall_rule_update_used 82
#define hotcall_ecall_backup_and_set_evictable 83


/* API untrusted functions to trusted inside the enclave */
struct match;
struct cls_rule;
struct heap_node;
struct match;
struct mf_subfield;
struct minimatch;
struct flow;
struct flow_wildcards;

/* A hash map. */
struct hmap {
    struct hmap_node **buckets; /* Must point to 'one' iff 'mask' == 0. */
    struct hmap_node *one;
    size_t mask;
    size_t n;
};

/* A hash map node, to be embedded inside the data structure being mapped. */
struct hmap_node {
    size_t hash;                /* Hash value. */
    struct hmap_node *next;     /* Next in linked list. */
};

struct list {
    struct list *prev;     /* Previous list element. */
    struct list *next;     /* Next list element. */
};


#define HOTCALL_MAX_ARG 25
typedef struct {
    int n_args;
    void *args[HOTCALL_MAX_ARG];
} argument_list;


typedef struct {
  size_t allocated_size;
  void *val;
} return_value;


typedef struct {
    struct cls_rule * cr;
    struct hmap_node hmap_node;
    struct hmap_node hmap_node_ut_crs;
    struct list list_node;
    int nr;
} cls_cache_entry;


#define PAGE_STATUS_FREE 0
#define PAGE_STATUS_ALLOCATED 1

#define ui8 "a"
#define ui8_c 'a'
#define ui16 "f"
#define ui16_c 'f'
#define ui32 "c"
#define ui32_c 'c'


struct page {
    uint8_t *bytes;
    size_t size;
    size_t status;
    uint8_t pending_deallocation;
};

typedef struct {
    struct page **pages;
    size_t  cap;
    uint8_t *allocated;
    size_t default_page_sz;
    sgx_spinlock_t spinlock;
} shared_memory;

typedef struct {
    size_t cap;
    // Mapping from Hash(flow + wc) to cls_cache_entry
    struct hmap entries;
    // When inserting a new entry in cache, kick out the entry in the beginning of this list.
    struct list lru_list;
    // Mapping from Hash(untrusted cr addr) to cls_cache_entry;
    struct hmap ut_crs;
    /*
        Memory region which the enclave will allocate hmap buckets when inserting new cache entries in "entries".
        It's neccasary to pass the enclave a region of memory in untrusted memory since the enclave cannot allocate memory in the untrusted part of memory.
        hmap entries has to be allocated in untrusted memory because otherwise it will not be accessible by the untrusted part of the application.
    */
    shared_memory shared_memory;
} flow_map_cache;

struct function_call {
    uint8_t id;
    argument_list args;
    void *return_value;
    char *fmt;
    bool async;
    struct list list_node;
};

struct hotcall {
    sgx_thread_mutex_t mutex;
    sgx_spinlock_t spinlock;
    sgx_thread_cond_t cond;
    bool run;
    bool running_function;
    bool is_done;
    bool sleeping;
    int timeout_counter;
    struct list ecall_queue;
};

struct shared_memory_ctx {
  struct hotcall hcall;
  flow_map_cache flow_cache;
};

#endif
