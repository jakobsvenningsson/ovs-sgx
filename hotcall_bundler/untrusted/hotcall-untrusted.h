#ifndef _H_HOTCALL_UNTRUSTED__
#define _H_HOTCALL_UNTRUSTED__

#include "stdbool.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <hotcall.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HCALL(sm_ctx_, f, async, ret, n_args, args) \
    struct ecall_queue_item *fc_; \
    fc_ = get_fcall(&sm_ctx_.pfc, hotcall_ ## f, ret, n_args, args); \
    list_insert(&sm_ctx.hcall.ecall_queue, &fc_->list_node); \
    if(!async) { \
        make_hotcall(&sm_ctx.hcall); \
    }
void hotcall_flush(struct shared_memory_ctx *sm_ctx);

bool
is_transaction_in_progress(struct hotcall *hcall);
int
first_call_of_transaction(struct hotcall *hcall);
void
hotcall_init(struct shared_memory_ctx *sm_ctx);
void
hotcall_flush(struct shared_memory_ctx *sm_ctx);
void
hotcall_bundle_begin(struct shared_memory_ctx *sm_ctx, int *transaction_error);
void
hotcall_bundle_end(struct shared_memory_ctx *sm_ctx);
void
hotcall_bundle_assert_false(struct preallocated_function_calls *pfc, int condition, int error_code, uint8_t cleanup_function);
void
hotcall_bundle_expected_value(struct preallocated_function_calls *pfc, int expected, int error_code, bool has_else);
void
hotcall_bundle_if_(struct preallocated_function_calls *pfc, int expected, int **conditions, char *type, int n_conditions, int if_len, int else_len);
void
hotcall_bundle_if_null_(struct preallocated_function_calls *pfc, void *condition, int if_len, int else_len);

inline void
make_hotcall(struct hotcall *hcall) {
    struct function_call *fcall, *next;
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

inline
struct ecall_queue_item * get_fcall(struct preallocated_function_calls *pfc, uint8_t f_id, void *ret, uint8_t n_args, void **args) {
    struct ecall_queue_item *item = &pfc->fcs[pfc->idx++];
    struct transaction_if *trans_if;
    switch(f_id) {
        case hotcall_transaction_guard:
            item->type = 1;
            struct transaction_assert *trans_assert = &item->call.ta;
            trans_assert->expected = *(int *) args[0];
            trans_assert->error = *(int *) args[1];
            trans_assert->transaction_error = (int *) args[2];
            trans_assert->has_else = *(bool *) args[3];
            break;
        case hotcall_transaction_if_null:
            item->type = 2;
            trans_if = &item->call.tif;
            trans_if->predicate_type = 0;
            trans_if->predicate.null_type.condition = (void *) args[0];
            trans_if->if_len = *(unsigned int *) args[1];
            trans_if->else_len = *(unsigned int *) args[2];
            break;
        case hotcall_transaction_if:
            item->type = 2;
            trans_if = &item->call.tif;
            trans_if->predicate_type = 1;
            trans_if->predicate.num_type.expected = *(int *) args[0];
            trans_if->predicate.num_type.n_conditions = *(int *) args[2];;
            memcpy(trans_if->predicate.num_type.conditions, args[1], 8 * trans_if->predicate.num_type.n_conditions);
            trans_if->if_len = *(int *) args[3];
            trans_if->else_len = *(int *) args[4];
            memcpy(trans_if->predicate.num_type.type, (char *) args[5], 5);
            break;
        default:
            item->type = 0;
            struct function_call *fcall = &item->call.fc;
            fcall->id = f_id;
            fcall->args.n_args = n_args;
            fcall->return_value = ret;
            memcpy(fcall->args.args, args, fcall->args.n_args * sizeof(void *));
            break;


    }
    if(pfc->idx == MAX_TS) {
        pfc->idx = 0;
    }
    return item;
}

inline size_t *
next_sizet(struct preallocated_function_calls *pfc, size_t x) {
    if(pfc->idx_sizet == MAX_TS) {
        pfc->idx_sizet = 0;
    }
    pfc->size_ts[pfc->idx_sizet] = x;
    return &pfc->size_ts[pfc->idx_sizet++];
}

inline uint8_t *
next_uint8(struct preallocated_function_calls *pfc, uint8_t x) {
    if(pfc->idx_uint8 == MAX_TS) {
        pfc->idx_uint8 = 0;
    }
    pfc->uint8_ts[pfc->idx_uint8] = x;
    return &pfc->uint8_ts[pfc->idx_uint8++];
}

inline unsigned int *
next_unsigned(struct preallocated_function_calls *pfc, unsigned int x) {
    if(pfc->idx_unsigned == MAX_TS) {
        pfc->idx_unsigned = 0;
    }
    pfc->unsigned_ts[pfc->idx_unsigned] = x;
    return &pfc->unsigned_ts[pfc->idx_unsigned++];
}

inline void *
next_voidptr(struct preallocated_function_calls *pfc, void *ptr) {
    if(pfc->idx_void == MAX_TS) {
        pfc->idx_void = 0;
    }
    pfc->void_ts[pfc->idx_void] = ptr;
    return pfc->void_ts[pfc->idx_void++];
}

inline void *
next_uint32(struct preallocated_function_calls *pfc, uint32_t x) {
    if(pfc->idx_uint32 == MAX_TS) {
        pfc->idx_uint32 = 0;
    }
    pfc->uint32_ts[pfc->idx_uint32] = x;
    return &pfc->uint32_ts[pfc->idx_uint32++];
}

inline void *
next_bool(struct preallocated_function_calls *pfc, bool b) {
    if(pfc->idx_bool == MAX_TS) {
        pfc->idx_bool = 0;
    }
    pfc->bool_ts[pfc->idx_bool] = b;
    return &pfc->bool_ts[pfc->idx_bool++];
}

inline void *
next_int(struct preallocated_function_calls *pfc, int x) {
    if(pfc->idx_int == MAX_TS) {
        pfc->idx_int = 0;
    }
    pfc->int_ts[pfc->idx_int] = x;
    return &pfc->int_ts[pfc->idx_int++];
}


#ifdef __cplusplus
}
#endif

#endif
