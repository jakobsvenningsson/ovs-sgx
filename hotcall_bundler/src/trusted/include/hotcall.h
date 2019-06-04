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
#define QUEUE_ITEM_TYPE_FOR_EACH 5
#define QUEUE_ITEM_TYPE_FOR_END 6
#define QUEUE_ITEM_TYPE_FOR_BEGIN 7
#define QUEUE_ITEM_TYPE_FILTER 8
#define QUEUE_ITEM_TYPE_DO_WHILE 9
#define QUEUE_ITEM_TYPE_WHILE_BEGIN 10
#define QUEUE_ITEM_TYPE_WHILE_END 11


#define HOTCALL_MAX_ARG 25

typedef struct {
    int n_args;
    void *args[HOTCALL_MAX_ARG];
} argument_list;

struct function_call {
    uint8_t id;
    argument_list args;
    void *return_value;
    bool async;
};


struct for_each_args {
    char *fmt;
    unsigned int n_params;
    void **params;
    unsigned int params_length;
};

struct immutable_function_argument {
    unsigned int n_params;
    char *fmt;
    void **params_in;
    unsigned int params_in_length;
    void **params_out;
    unsigned int params_out_length;
};


enum variable_type { FUNCTION_TYPE, VARIABLE_TYPE, POINTER_TYPE };
#define MAX_N_VARIABLES 5

struct predicate_variable {
    const void *val;
    enum variable_type type;
    char fmt;
};

struct predicate {
    bool expected;
    char *fmt;
    uint8_t n_variables;
    struct predicate_variable *variables;
};


struct if_args {
    unsigned int then_branch_len;
    unsigned int else_branch_len;
    struct predicate predicate;
    bool return_if_false;
};

struct for_args {
        unsigned int n_iters;
        unsigned int n_rows;
};


struct while_args {
    struct predicate predicate;
    unsigned int n_rows;
};
/*
struct do_while_args {
    unsigned int params_n;
    void **params_in;
    char *params_fmt;

    unsigned int condition_n;
    void **condition_params;
    char *condition_fmt;

    struct numertic_type condition;

};*/

struct transaction_if {
    struct if_args *args;
    //unsigned int n_clauses;
    /*struct predicate_ predicate;
    //union predicate predicate;
    unsigned int else_len;
    unsigned int if_len;*/
};

struct transaction_filter {
    uint8_t f;
    struct immutable_function_argument *args;
    unsigned int *n_iter;
    unsigned int n_params;
    unsigned int *filtered_length;
    void **params_in;
    void **params_out;
    char *fmt;
};

struct transaction_do_while {
    uint8_t f;
    struct do_while_args *args;
};

struct transaction_for_each {
    uint8_t f;
    struct for_each_args *args;
    /*unsigned int *n_iter;
    unsigned int n_params;
    void **params;
    char *fmt;*/
};

struct transaction_for_start {
    struct for_args *args;
};

struct transaction_for_end {
    struct for_args *args;
};

struct transaction_while_start {
    struct while_args *args;
};

struct transaction_while_end {
    struct while_args *args;
};

union fcall {
    struct function_call fc;
    struct transaction_if tif;
    struct transaction_for_start for_s;
    struct transaction_for_end for_e;
    struct transaction_for_each tor;
    struct transaction_filter fi;
    struct transaction_do_while dw;
    struct transaction_while_start while_s;
    struct transaction_while_end while_e;

};

struct ecall_queue_item {
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
    struct ecall_queue_item *ecall_queue[MAX_FCS];
    unsigned int queue_length;
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
