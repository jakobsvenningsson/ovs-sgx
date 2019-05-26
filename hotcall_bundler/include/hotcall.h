#ifndef _H_LIB_HOTCALL_
#define _H_LIB_HOTCALL_

#include "/home/jakob/ovs-sgx/hotcall_bundler/lib/list.h"

#include <sgx_spinlock.h>
#include <sgx_thread.h>

#define HOTCALL_MAX_ARG 25

#define hotcall_transaction_if_null 200
#define hotcall_transaction_assert_true_ 201
#define hotcall_transaction_assert_false_ 202
#define hotcall_transaction_guard 203
#define hotcall_transaction_if 204


typedef struct {
    int n_args;
    void *args[HOTCALL_MAX_ARG];
} argument_list;


typedef struct {
  size_t allocated_size;
  void *val;
} return_value;


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

#define MAX_FCS 200
#define MAX_TS 200

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


#define MAX_SPINLOCK_JOBS 5

struct shared_memory_ctx {
  struct hotcall hcall;
  void *custom_object_ptr[5];
  struct preallocated_function_calls pfc;
};

struct hotcall_config {
    void (*execute_function)(struct function_call *);
    void (*spin_lock_tasks[MAX_SPINLOCK_JOBS])();
    unsigned int spin_lock_task_count[MAX_SPINLOCK_JOBS];
    unsigned int spin_lock_task_timeouts[MAX_SPINLOCK_JOBS];
    unsigned int n_spinlock_jobs;
};

#endif
