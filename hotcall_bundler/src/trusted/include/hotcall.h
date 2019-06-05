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
#include "hotcall_if.h"
#include "hotcall_predicate.h"

#define QUEUE_ITEM_TYPE_IF 0
#define QUEUE_ITEM_TYPE_IF_NULL 1
#define QUEUE_ITEM_TYPE_GUARD 2
#define QUEUE_ITEM_TYPE_DESTROY 3
#define QUEUE_ITEM_TYPE_FUNCTION 4
#define QUEUE_ITEM_TYPE_FOR_EACH 5
#define QUEUE_ITEM_TYPE_FOR_END 6
#define QUEUE_ITEM_TYPE_FOR_BEGIN 7
#define QUEUE_ITEM_TYPE_FILTER 8
#define QUEUE_ITEM_TYPE_DO_WHILE 9
#define QUEUE_ITEM_TYPE_WHILE_BEGIN 10
#define QUEUE_ITEM_TYPE_WHILE_END 11
#define QUEUE_ITEM_TYPE_MAP 12

#define MAX_FCS 200
#define MAX_TS 200
#define MAX_N_VARIABLES 5

union hcall {
    struct hotcall_function fc;
    struct hotcall_if tif;
    struct hotcall_for_start for_s;
    struct hotcall_for_end for_e;
    struct hotcall_for_each tor;
    struct hotcall_filter fi;
    struct hotcall_do_while dw;
    struct hotcall_while_start while_s;
    struct hotcall_while_end while_e;
    struct hotcall_map ma;
};


struct ecall_queue_item {
    uint8_t type;
    union hcall call;
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
    struct ecall_queue_item *ecall_queue[MAX_FCS];
    unsigned int queue_length;
    bool hotcall_in_progress;
};


struct preallocated_function_calls {
    struct ecall_queue_item fcs[MAX_FCS];
    void *args[MAX_FCS][HOTCALL_MAX_ARG];
    size_t idx;
    size_t len;

    size_t idx_uint8;
    uint8_t uint8_ts[MAX_TS];

    size_t idx_sizet;
    size_t size_ts[MAX_TS];

    size_t idx_unsigned;
    unsigned int unsigned_ts[MAX_TS];

    size_t idx_void;
    void *void_ts[MAX_TS];

    size_t idx_uint32;
    uint32_t uint32_ts[MAX_TS];

    size_t idx_bool;
    bool bool_ts[MAX_TS];

    size_t idx_int;
    int int_ts[MAX_TS];
};

struct shared_memory_ctx {
  struct hotcall hcall;
  void *custom_object_ptr[MAX_N_VARIABLES];
  struct preallocated_function_calls pfc;
};


#endif
