#ifndef _H_HOTCALL_UNTRUSTED__
#define _H_HOTCALL_UNTRUSTED__

#include "stdbool.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <hotcall.h>
#include "sgx_eid.h"


#ifdef __cplusplus
extern "C" {
#endif


#ifdef BATCHING
#define ASYNC(SM_CTX, X) X || is_transaction_in_progress(&(SM_CTX)->hcall)
#else
#define ASYNC(SM_CTX, X) false || is_transaction_in_progress(&(SM_CTX)->hcall)
#endif
//list_insert(&(SM_CTX)->hcall.ecall_queue, &QI->list_node);

#define HCALL(SM_CTX, F, _ASYNC, RET, N_ARGS, ARGS) \
    (SM_CTX)->hcall.ecall_queue[(SM_CTX)->hcall.queue_length++] = \
        get_fcall(&(SM_CTX)->pfc, QUEUE_ITEM_TYPE_FUNCTION, hotcall_ ## F, RET, N_ARGS, ARGS); \
    if(ASYNC(SM_CTX, _ASYNC) != 1) { \
        make_hotcall(&(SM_CTX)->hcall); \
    }

//list_insert(&(SM_CTX)->hcall.ecall_queue, &QI->list_node);
#define HCALL_CONTROL(SM_CTX, TYPE, _ASYNC, N_ARGS, ARGS) \
    (SM_CTX)->hcall.ecall_queue[(SM_CTX)->hcall.queue_length++] = \
        get_fcall(&(SM_CTX)->pfc, QUEUE_ITEM_TYPE_ ## TYPE, 0, NULL, N_ARGS, ARGS); \
    if(ASYNC(SM_CTX, _ASYNC) != 1) { \
        make_hotcall(&(SM_CTX)->hcall); \
    }

#define ASSERT_CATCH(F, EXPECTED, ERROR, ELSE_F) \
    F; \
    hotcall_transaction_expected_value(EXPECTED, ERROR, true); \
    ELSE_F

#define ASSERT(F, EXPECTED, ERROR) \
    F; \
    hotcall_transaction_expected_value(EXPECTED, ERROR, false)

#define IF(SM_CTX, EXPECTED, IF_LEN, ELSE_LEN, FMT, N_COND, ...) \
    { \
        int expected = EXPECTED, if_len = IF_LEN, else_len = ELSE_LEN, n_cond = N_COND; \
        const unsigned int *conditions[N_COND] = { __VA_ARGS__ }; \
        hotcall_bundle_if_(SM_CTX, &expected, conditions, FMT, &n_cond, &if_len, &else_len); \
    }


#define IF_NOT_NULL(CONDITION, IF_LEN, ELSE_LEN) \
    hotcall_transaction_if_null_(CONDITION, IF_LEN, ELSE_LEN)

#define THEN(...) __VA_ARGS__
#define ELSE(...) __VA_ARGS__
#define CONDITION(...) __VA_ARGS__


//void hotcall_flush(struct shared_memory_ctx *sm_ctx);


extern inline bool
is_transaction_in_progress(struct hotcall *hcall) {
    return hcall->transaction_in_progress;
}

void
hotcall_init(struct shared_memory_ctx *sm_ctx, sgx_enclave_id_t _global_eid);
void
hotcall_bundle_flush(struct shared_memory_ctx *sm_ctx);
/*void
hotcall_bundle_begin(struct shared_memory_ctx *sm_ctx, int *transaction_error);
void
hotcall_bundle_end(struct shared_memory_ctx *sm_ctx);*/
void
hotcall_bundle_assert_false(struct preallocated_function_calls *pfc, int condition, int error_code, uint8_t cleanup_function);
void
hotcall_bundle_expected_value(struct preallocated_function_calls *pfc, int expected, int error_code, bool has_else);
void
hotcall_bundle_if_null_(struct preallocated_function_calls *pfc, void *condition, int if_len, int else_len);
void
hotcall_destroy(struct shared_memory_ctx *sm_ctx);

extern inline void
hotcall_bundle_begin(struct shared_memory_ctx *sm_ctx, int *transaction_error) {
    //transaction_err = transaction_error;
    sm_ctx->hcall.transaction_in_progress = true;
}

extern inline void
hotcall_bundle_end(struct shared_memory_ctx *sm_ctx) {
    hotcall_bundle_flush(sm_ctx);
    //transaction_err = NULL;
    sm_ctx->hcall.transaction_in_progress = false;
}


extern inline
struct ecall_queue_item * get_fcall(struct preallocated_function_calls *pfc, uint8_t f_type, uint8_t f_id, void *ret, uint8_t n_args, void **args) {
    struct ecall_queue_item *item = &pfc->fcs[pfc->idx++];
    item->type = f_type;
    switch(item->type) {
        case QUEUE_ITEM_TYPE_GUARD:
        {
            struct transaction_assert *trans_assert;
            trans_assert = &item->call.ta;
            trans_assert->expected = *(int *) args[0];
            trans_assert->error = *(int *) args[1];
            trans_assert->transaction_error = (int *) args[2];
            trans_assert->has_else = *(bool *) args[3];
            break;
        }
        case QUEUE_ITEM_TYPE_IF_NULL:
        {
            /*struct transaction_if *trans_if;
            trans_if = &item->call.tif;
            trans_if->predicate_type = 0;
            trans_if->predicate.null_type.condition = (void *) args[0];
            trans_if->if_len = *(unsigned int *) args[1];
            trans_if->else_len = *(unsigned int *) args[2];*/
            break;
        }
        case QUEUE_ITEM_TYPE_IF:
        {
            struct transaction_if *trans_if;
            trans_if = &item->call.tif;
            trans_if->predicate_type = 1;
            //memcpy(&trans_if->predicate.num_type, args, 8 * 4);
            trans_if->predicate.num_type.expected = (unsigned int *) args[0]; //*(int *) args[0];
            trans_if->predicate.num_type.conditions = (void **) args[1];
            trans_if->predicate.num_type.n_conditions = (unsigned int *) args[2];//*(int *) args[2];;
            trans_if->predicate.num_type.type = (char *) args[3];

            trans_if->else_len =  (unsigned int *) args[4]; //*(int *) args[4];
            trans_if->if_len = (unsigned int *) args[5]; //*(int *) args[3];

            //memcpy(trans_if->predicate.num_type.conditions, args[1], 8 * trans_if->predicate.num_type.n_conditions
            //memcpy(trans_if->predicate.num_type.type, (char *) args[5], 5);
            break;
        }
        case QUEUE_ITEM_TYPE_DESTROY:
            break;
        case QUEUE_ITEM_TYPE_FUNCTION:
        {
            struct function_call *fcall;
            fcall = &item->call.fc;
            fcall->id = f_id;
            fcall->args.n_args = n_args;
            fcall->return_value = ret;
            memcpy(fcall->args.args, args, fcall->args.n_args * sizeof(void *));
            break;
        }
        default:
            printf("unknown queue type\n");
    }
    if(pfc->idx == MAX_TS) {
        pfc->idx = 0;
    }
    return item;
}

extern inline void
make_hotcall(struct hotcall *hcall) {
    hcall->is_done  = false;
    hcall->run      = true;
    while (1) {
        sgx_spin_lock(&hcall->spinlock);
        if (hcall->is_done) {
            sgx_spin_unlock(&hcall->spinlock);
            break;
        }
        sgx_spin_unlock(&hcall->spinlock);
        for (int i = 0; i < 3; ++i) {
            __asm
            __volatile(
              "pause"
            );
        }
    }
}

extern inline size_t *
next_sizet(struct preallocated_function_calls *pfc, size_t x) {
    if(pfc->idx_sizet == MAX_TS) {
        pfc->idx_sizet = 0;
    }
    pfc->size_ts[pfc->idx_sizet] = x;
    return &pfc->size_ts[pfc->idx_sizet++];
}

extern inline uint8_t *
next_uint8(struct preallocated_function_calls *pfc, uint8_t x) {
    if(pfc->idx_uint8 == MAX_TS) {
        pfc->idx_uint8 = 0;
    }
    pfc->uint8_ts[pfc->idx_uint8] = x;
    return &pfc->uint8_ts[pfc->idx_uint8++];
}

extern inline unsigned int *
next_unsigned(struct preallocated_function_calls *pfc, unsigned int x) {
    if(pfc->idx_unsigned == MAX_TS) {
        pfc->idx_unsigned = 0;
    }
    pfc->unsigned_ts[pfc->idx_unsigned] = x;
    return &pfc->unsigned_ts[pfc->idx_unsigned++];
}

extern inline void *
next_voidptr(struct preallocated_function_calls *pfc, void *ptr) {
    if(pfc->idx_void == MAX_TS) {
        pfc->idx_void = 0;
    }
    pfc->void_ts[pfc->idx_void] = ptr;
    return pfc->void_ts[pfc->idx_void++];
}

extern inline void *
next_uint32(struct preallocated_function_calls *pfc, uint32_t x) {
    if(pfc->idx_uint32 == MAX_TS) {
        pfc->idx_uint32 = 0;
    }
    pfc->uint32_ts[pfc->idx_uint32] = x;
    return &pfc->uint32_ts[pfc->idx_uint32++];
}

extern inline void *
next_bool(struct preallocated_function_calls *pfc, bool b) {
    if(pfc->idx_bool == MAX_TS) {
        pfc->idx_bool = 0;
    }
    pfc->bool_ts[pfc->idx_bool] = b;
    return &pfc->bool_ts[pfc->idx_bool++];
}

extern inline void *
next_int(struct preallocated_function_calls *pfc, int x) {
    if(pfc->idx_int == MAX_TS) {
        pfc->idx_int = 0;
    }
    pfc->int_ts[pfc->idx_int] = x;
    return &pfc->int_ts[pfc->idx_int++];
}

/*
extern inline void
make_hcall(struct shared_memory_ctx *sm_ctx, struct ecall_queue_item *queue_item, uint8_t f, bool async, void *ret, uint8_t n_args, void **args) {
  queue_item = get_fcall(&sm_ctx->pfc, f, ret, n_args, args);
  list_insert(&sm_ctx->hcall.ecall_queue, &queue_item->list_node);
  if(!async) {
      make_hotcall(&sm_ctx->hcall);
  }
}*/

extern inline void
hotcall_bundle_if_(struct shared_memory_ctx *sm_ctx, int *expected, const unsigned int *conditions[], char *type, int *n_conditions, int *if_len, int *else_len) {
    void **args = sm_ctx->pfc.args[sm_ctx->pfc.idx];
    args[0] = expected;//next_int(&sm_ctx->pfc, expected);
    args[1] = (void **) conditions;
    args[2] = n_conditions; //next_int(&sm_ctx->pfc, n_conditions);
    args[3] = type;
    args[4] = else_len;//next_int(&sm_ctx->pfc, else_len);
    args[5] = if_len;//next_int(&sm_ctx->pfc, if_len);
    HCALL_CONTROL(sm_ctx, IF, false, 6, args);
}

#ifdef __cplusplus
}
#endif

#endif
