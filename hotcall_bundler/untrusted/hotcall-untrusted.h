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
/*
#define ASSERT_CATCH(F, EXPECTED, ERROR, ELSE_F) \
    F; \
    hotcall_transaction_expected_value(EXPECTED, ERROR, true); \
    ELSE_F

#define ASSERT(F, EXPECTED, ERROR) \
    F; \
    hotcall_transaction_expected_value(EXPECTED, ERROR, false)
*/
/*
#define IF(SM_CTX, EXPECTED, IF_LEN, ELSE_LEN, FMT, CLAUSE_TYPE, N_COND, ...) \
    const void *conditions[N_COND] = { __VA_ARGS__ }; \
    hotcall_bundle_if_(SM_CTX, EXPECTED, conditions, FMT, N_COND, IF_LEN, ELSE_LEN, CLAUSE_TYPE);*/


#define IF(SM_CTX, ARGS) \
    hotcall_bundle_if_(SM_CTX, ARGS);

/*
#define IF_NOT_NULL(CONDITION, IF_LEN, ELSE_LEN) \
    hotcall_transaction_if_null_(CONDITION, IF_LEN, ELSE_LEN)*/

#define THEN(...) __VA_ARGS__
#define ELSE(...) __VA_ARGS__

//#define CONDITION(...) __VA_ARGS__


#define FOR_EACH(SM_CTX, FUNCTION, ITERS, N_PARAMS, PARAMS, FMT) \
    hotcall_bundle_for_each(SM_CTX, hotcall_ ## FUNCTION, ITERS, N_PARAMS, PARAMS, FMT)

#define FILTER(SM_CTX, FUNCTION, N_PARAMS, PARAMS, ITERS, LEN_FILTERED, FILTERED_OUT, FMT) \
    hotcall_bundle_filter(SM_CTX, hotcall_ ## FUNCTION, N_PARAMS, PARAMS, ITERS, LEN_FILTERED, FILTERED_OUT, FMT)
/*
#define FOR_EACH(SM_CTX, FUNCTION, ARGS) \
    hotcall_bundle_for_each(SM_CTX, hotcall_ ## FUNCTION, ARGS)

#define FILTER(SM_CTX, FUNCTION, ARGS) \
    hotcall_bundle_filter(SM_CTX, hotcall_ ## FUNCTION, ARGS)*/


#define DO_WHILE(SM_CTX, F, ARGS) \
    hotcall_bundle_for_each((SM_CTX), hotcall_ ## F, ARGS)

#define BEGIN_FOR(SM_CTX, N_ITERS, N_ROWS) \
    hotcall_bundle_for_begin(SM_CTX, N_ITERS, N_ROWS)
#define END_FOR(SM_CTX, N_ROWS) \
    hotcall_bundle_for_end(SM_CTX, N_ROWS)

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
            /*
            struct transaction_assert *trans_assert;
            trans_assert = &item->call.ta;
            trans_assert->expected = *(int *) args[0];
            trans_assert->error = *(int *) args[1];
            trans_assert->transaction_error = (int *) args[2];
            trans_assert->has_else = *(bool *) args[3];*/
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
            //trans_if->predicate_type = 1;
            //memcpy(&trans_if->predicate.num_type, args, 8 * 4);
            //trans_if->predicate.num_type.expected = (unsigned int *) args[0]; //*(int *) args[0];
            //trans_if->predicate.num_type.conditions = (void **) args[1];
            //trans_if->predicate.num_type.n_conditions = (unsigned int *) args[2];//*(int *) args[2];;
            //trans_if->predicate.num_type.fmt = (char *) args[3];

            //trans_if->else_len =  (unsigned int *) args[4]; //*(int *) args[4];
            //trans_if->if_len = (unsigned int *) args[5]; //*(int *) args[3];

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
hotcall_bundle_if_(struct shared_memory_ctx *sm_ctx, struct if_args *args) {
//hotcall_bundle_if_(struct shared_memory_ctx *sm_ctx, bool expected, const void *clauses[], char *fmt, int n_clauses, int if_len, int else_len, char *clause_type) {
    //void **args = sm_ctx->pfc.args[sm_ctx->pfc.idx];
    //args[0] = next_int(&sm_ctx->pfc, expected);
    //args[1] = (void **) clauses;
    //args[2] = next_int(&sm_ctx->pfc, n_clauses);
    //args[3] = type;
    //args[4] = next_int(&sm_ctx->pfc, else_len);
    //args[5] = next_int(&sm_ctx->pfc, if_len);
    //HCALL_CONTROL(sm_ctx, IF, false, 6, args);


    struct ecall_queue_item *item;
    item = &sm_ctx->pfc.fcs[sm_ctx->pfc.idx++];
    item->type = QUEUE_ITEM_TYPE_IF;
    if(sm_ctx->pfc.idx == MAX_TS) {
        sm_ctx->pfc.idx = 0;
    }
    struct transaction_if *tif;
    tif = &item->call.tif;
    tif->args = args;

    /*tif->else_len = else_len;
    tif->if_len = if_len;
    tif->predicate.expected = expected;
    tif->predicate.fmt = fmt;
    tif->predicate.n_variables = n_clauses;

    for(int i = 0; i < n_clauses; ++i) {
        tif->predicate.variables[i].val = clauses[i];
        switch(clause_type[i]) {
            case 'f':
                tif->predicate.variables[i].type = FUNCTION_TYPE;
                break;
            case 'p':
                tif->predicate.variables[i].type = POINTER_TYPE;
                break;
            case 'v':
                tif->predicate.variables[i].type = VARIABLE_TYPE;
                break;

            default:
                printf("unknown clause type.\n");
        }
    }*/

    sm_ctx->hcall.ecall_queue[sm_ctx->hcall.queue_length++] = item;
}

extern inline void
//hotcall_bundle_filter(struct shared_memory_ctx *sm_ctx, uint8_t f, struct immutable_function_argument *args) {
hotcall_bundle_filter(struct shared_memory_ctx *sm_ctx, uint8_t f, int n_params, void **params, unsigned int *n_iter, unsigned int *filtered_length, void **filtered_params, char *fmt) {
    struct ecall_queue_item *item;
    item = &sm_ctx->pfc.fcs[sm_ctx->pfc.idx++];
    item->type = QUEUE_ITEM_TYPE_FILTER;
    if(sm_ctx->pfc.idx == MAX_TS) {
        sm_ctx->pfc.idx = 0;
    }
    struct transaction_filter *fi;
    fi = &item->call.fi;
    fi->f = f;
    //fi->args = args;
    fi->filtered_length = filtered_length;
    fi->n_iter = n_iter;
    fi->n_params = n_params;
    fi->params_in = params;
    fi->params_out = filtered_params;
    fi->fmt = fmt;

    sm_ctx->hcall.ecall_queue[sm_ctx->hcall.queue_length++] = item;
}

extern inline void
hotcall_bundle_do_while(struct shared_memory_ctx *sm_ctx, uint8_t f, struct do_while_args *do_while_args) {
    struct ecall_queue_item *item;
    item = &sm_ctx->pfc.fcs[sm_ctx->pfc.idx++];
    item->type = QUEUE_ITEM_TYPE_DO_WHILE;
    if(sm_ctx->pfc.idx == MAX_TS) {
        sm_ctx->pfc.idx = 0;
    }
    struct transaction_do_while *dw;
    dw = &item->call.dw;
    dw->args = do_while_args;

    sm_ctx->hcall.ecall_queue[sm_ctx->hcall.queue_length++] = item;
}

extern inline void
//hotcall_bundle_for_each(struct shared_memory_ctx *sm_ctx, uint8_t f, struct immutable_function_argument *args) {
hotcall_bundle_for_each(struct shared_memory_ctx *sm_ctx, uint8_t f, unsigned int *n, int n_params, void **params, char *fmt) {
    struct ecall_queue_item *item;
    item = &sm_ctx->pfc.fcs[sm_ctx->pfc.idx++];
    item->type = QUEUE_ITEM_TYPE_FOR_EACH;
    if(sm_ctx->pfc.idx == MAX_TS) {
        sm_ctx->pfc.idx = 0;
    }
    struct transaction_for_each *tor;
    tor = &item->call.tor;
    tor->fmt = fmt;
    tor->f = f;
    //tor->args = args;
    tor->n_iter = n;
    tor->n_params = n_params;
    tor->params = params;
    sm_ctx->hcall.ecall_queue[sm_ctx->hcall.queue_length++] = item;
}

extern inline void
hotcall_bundle_for_begin(struct shared_memory_ctx *sm_ctx, unsigned int n_iters, unsigned int n_rows) {
    struct ecall_queue_item *item;
    item = &sm_ctx->pfc.fcs[sm_ctx->pfc.idx++];
    item->type = QUEUE_ITEM_TYPE_FOR_BEGIN;
    if(sm_ctx->pfc.idx == MAX_TS) {
        sm_ctx->pfc.idx = 0;
    }
    struct transaction_for_start *for_s;
    for_s = &item->call.for_s;
    for_s->n_iters = n_iters;
    for_s->n_rows = n_rows;
    sm_ctx->hcall.ecall_queue[sm_ctx->hcall.queue_length++] = item;

}

extern inline void
hotcall_bundle_for_end(struct shared_memory_ctx *sm_ctx, unsigned int n_rows) {
    struct ecall_queue_item *item;
    item = &sm_ctx->pfc.fcs[sm_ctx->pfc.idx++];
    item->type = QUEUE_ITEM_TYPE_FOR_END;
    if(sm_ctx->pfc.idx == MAX_TS) {
        sm_ctx->pfc.idx = 0;
    }
    struct transaction_for_end *for_e;
    for_e = &item->call.for_e;
    for_e->n_rows = n_rows;
    sm_ctx->hcall.ecall_queue[sm_ctx->hcall.queue_length++] = item;
}


extern inline void
hotcall_bundle_for(struct shared_memory_ctx *sm_ctx, uint8_t *fcs, int n, int n_params, void **params) {
    /*struct ecall_queue_item *item
    item = &sm_ctx->pfc.fcs[sm_ctx->pfc.idx++];
    if(sm_ctx->pfc.idx == MAX_TS) {
        sm_ctx->pfc.idx = 0;
    }
    item->type = QUEUE_ITEM_TYPE_FOR_START;
    struct transaction_for_start *for_s;
    for_s = &item->call.for_s;
    //tor->f = f;
    for_s->n_iter = n;
    for_s->n_params = n_params;
    for_s->params = params;
    sm_ctx->hcall.ecall_queue[sm_ctx->hcall.queue_length++] = item;


    struct function_call *fc;
    for(int i = 0; i < 1; ++i) {
        item = &sm_ctx->pfc.fcs[sm_ctx->pfc.idx++];
        if(sm_ctx->pfc.idx == MAX_TS) {
            sm_ctx->pfc.idx = 0;
        }
        fc = &item->call.fc;
        fc->id = fcs[i];
        fc->args.n_args = 1;
        fc->return_value = NULL;
        for(int j = 0; j < n_params; ++j) {
            fc->args.args[j] = (int *) params[j] + i;
        }
        sm_ctx->hcall.ecall_queue[sm_ctx->hcall.queue_length++] = item;
    }

    item = &sm_ctx->pfc.fcs[sm_ctx->pfc.idx++];
    if(sm_ctx->pfc.idx == MAX_TS) {
        sm_ctx->pfc.idx = 0;
    }
    item->type = QUEUE_ITEM_TYPE_FOR_END;
    struct transaction_for_end *for_e;
    for_e = &item->call.for_e;
    for_e->n_rows = 1;
    sm_ctx->hcall.ecall_queue[sm_ctx->hcall.queue_length++] = item;*/
}

#ifdef __cplusplus
}
#endif

#endif
