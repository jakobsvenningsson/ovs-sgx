#ifndef _H_LIB_HOTCALL_
#define _H_LIB_HOTCALL_

#include <sgx_spinlock.h>
#include <sgx_thread.h>
#include <stdbool.h>
#include <string.h>

#include "hotcall_do_while.h"
#include "hotcall_while.h"
#include "hotcall_for.h"
#include "hotcall_for_each.h"
#include "hotcall_filter.h"
#include "hotcall_map.h"
#include "hotcall_reduce.h"
#include "hotcall_if.h"
#include "hotcall_error.h"
#include "hotcall_assign_variable.h"
#include "hotcall_function.h"
#include "hotcall_assert.h"
#include "hotcall_cache.h"


#define QUEUE_ITEM_TYPE_IF_NULL 1
#define QUEUE_ITEM_TYPE_DESTROY 3
#define QUEUE_ITEM_TYPE_FUNCTION 4
#define QUEUE_ITEM_TYPE_FOR_EACH 5
#define QUEUE_ITEM_TYPE_FOR_BEGIN 7
#define QUEUE_ITEM_TYPE_FILTER 8
#define QUEUE_ITEM_TYPE_DO_WHILE 9
#define QUEUE_ITEM_TYPE_WHILE_BEGIN 10
#define QUEUE_ITEM_TYPE_MAP 12
#define QUEUE_ITEM_TYPE_ERROR 13
#define QUEUE_ITEM_TYPE_REDUCE 14
#define QUEUE_ITEM_TYPE_IF_ELSE 16
#define QUEUE_ITEM_TYPE_ASSIGN_VAR 17
#define QUEUE_ITEM_TYPE_ASSIGN_PTR 18
#define QUEUE_ITEM_TYPE_FOR_END 19
#define QUEUE_ITEM_TYPE_WHILE_END 20
#define QUEUE_ITEM_TYPE_IF 21
#define QUEUE_ITEM_TYPE_IF_END 22
#define QUEUE_ITEM_TYPE_ASSERT 23
#define QUEUE_ITEM_TYPE_ASSERT_FALSE 24

#define MAX_FCS 200
#define MAX_TS 200
#define MAX_N_VARIABLES 5

union hcall {
    struct hotcall_if tif;
    struct hotcall_for_start for_s;
    struct hotcall_for_each tor;
    struct hotcall_filter fi;
    struct hotcall_do_while dw;
    struct hotcall_while_start while_s;
    struct hotcall_map ma;
    struct hotcall_error err;
    struct hotcall_reduce re;
    struct hotcall_function fc;
    struct hotcall_assign_variable var;
    struct hotcall_assign_pointer ptr;
    struct hotcall_if_else tife;
    struct hotcall_assert as;
};

struct ecall_queue_item {
    uint8_t type;
    union hcall call;
    struct ecall_queue_item *next;
    struct ecall_queue_item *prev;
};

struct hotcall_batch {
    struct ecall_queue_item *queue; //[MAX_FCS];
    struct ecall_queue_item *top;
    unsigned int queue_len;
    int error;
    bool ignore_hcalls;
};

struct hotcall {
    sgx_thread_mutex_t mutex;
    sgx_spinlock_t spinlock;
    int timeout_counter;
    struct hotcall_batch *batch;
    struct ecall_queue_item *ecall;
    int error;
    bool sleeping;
    bool hotcall_in_progress;
    bool is_inside_chain;
    bool is_done;
    bool run;

};

struct memoize {
    unsigned int function_cache_size[256];
    struct function_cache_ctx *functions[256];
    unsigned int max_n_function_caches;

};


struct shared_memory_ctx {
    struct hotcall hcall;
    char pad[64];
    struct memoize mem;
    void *custom_object_ptr[MAX_N_VARIABLES];
};

#endif
