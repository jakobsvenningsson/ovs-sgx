#ifndef _SGX_COMMON_H
#define _SGX_COMMON_H

#include <sgx_spinlock.h>
#include <stdbool.h>
#include <stdarg.h>
#include <sgx_thread.h>
#include "/home/jakob/ovs-sgx/OFTonSGX/trusted/lib/list.h"


/* API untrusted functions to trusted inside the enclave */
struct match;
struct cls_rule;
struct heap_node;
struct match;
struct mf_subfield;
struct minimatch;
struct flow;
struct flow_wildcards;
struct ofproto_table_settings;


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


/* Configuration of OpenFlow tables. */
struct ofproto_table_settings {
    char *name;                 /* Name exported via OpenFlow or NULL. */
    unsigned int max_flows;     /* Maximum number of flows or UINT_MAX. */

    /* These members determine the handling of an attempt to add a flow that
     * would cause the table to have more than 'max_flows' flows.
     *
     * If 'groups' is NULL, overflows will be rejected with an error.
     *
     * If 'groups' is nonnull, an overflow will cause a flow to be removed.
     * The flow to be removed is chosen to give fairness among groups
     * distinguished by different values for the subfields within 'groups'. */
    struct mf_subfield *groups;
    size_t n_groups;
};

/* HOTCALL STRUCTURES */
/*
#define HOTCALL_MAX_ARG 25

typedef struct {
    int n_args;
    void *args[HOTCALL_MAX_ARG];
} argument_list;


typedef struct {
  size_t allocated_size;
  void *val;
} return_value;*/

typedef struct {
    struct cls_rule * cr;
    struct hmap_node hmap_node;
    struct hmap_node hmap_node_ut_crs;
    struct list list_node;
    int nr;
} cls_cache_entry;



/* SHARED MEMORY */

#define PAGE_STATUS_FREE 0
#define PAGE_STATUS_ALLOCATED 1

/*
#define ui8 "a"
#define ui8_c 'a'
#define ui16 "f"
#define ui16_c 'f'
#define ui32 "c"
#define ui32_c 'c'*/


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

/* CACHE */

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

/*
struct function_call {
    uint8_t id;
    argument_list args;
    void *return_value;
    char *fmt;
    bool async;
};

struct transaction_assert {
    int expected;
    int *transaction_error;
    int error;
    bool has_else;
};

struct numertic_type {
    void *conditions[5];
    char type[5];

    unsigned int n_clauses;
    unsigned int n_conditions;
    unsigned int expected;
};

struct null_type {
    void *condition;
};

union predicate {
    struct null_type null_type;
    struct numertic_type num_type;
};

struct transaction_if {
    uint8_t predicate_type;
    union predicate predicate;
    unsigned int else_len;
    unsigned int if_len;
};

union fcall {
    struct transaction_assert ta;
    struct function_call fc;
    struct transaction_if tif;
};

struct ecall_queue_item {
    struct list list_node;
    uint8_t type;
    union fcall call;
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

    bool transaction_in_progress;
    int first_call_of_transaction;
};

struct shared_memory_ctx {
  struct hotcall hcall;
  flow_map_cache flow_cache;
};*/

#endif
