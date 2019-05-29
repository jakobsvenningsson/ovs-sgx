#ifndef _H_LIB_HOTCALL_
#define _H_LIB_HOTCALL_

#include <sgx_spinlock.h>
#include <sgx_thread.h>
#include <stdbool.h>
#include <string.h>

#define QUEUE_ITEM_TYPE_IF 0
#define QUEUE_ITEM_TYPE_IF_NULL 1
#define QUEUE_ITEM_TYPE_GUARD 2
#define QUEUE_ITEM_TYPE_DESTROY 3
#define QUEUE_ITEM_TYPE_FUNCTION 4


#define HOTCALL_MAX_ARG 25

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
    unsigned int *expected;
    void **conditions;
    unsigned int *n_conditions;
    char *type;

    unsigned int *n_clauses;
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
    unsigned int *else_len;
    unsigned int *if_len;
};

struct transaction_for {
    struct function_call *fc;
    unsigned int n;
    argument_list *args[];

}

union fcall {
    struct transaction_assert ta;
    struct function_call fc;
    struct transaction_if tif;
    struct transaction_for tor;
};

struct ecall_queue_item {
    //struct list list_node;
    uint8_t type;
    union fcall call;
};


#define MAX_FCS 200
#define MAX_TS 200

struct hotcall {
    sgx_thread_mutex_t mutex;
    sgx_spinlock_t spinlock;
    sgx_thread_cond_t cond;
    bool run;
    bool running_function;
    bool is_done;
    bool sleeping;
    int timeout_counter;
    //struct list ecall_queue;
    struct ecall_queue_item *ecall_queue[MAX_FCS];
    uint8_t queue_length;

    bool transaction_in_progress;
    int first_call_of_transaction;
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
