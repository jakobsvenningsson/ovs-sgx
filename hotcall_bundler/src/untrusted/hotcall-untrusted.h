#ifndef _H_HOTCALL_UNTRUSTED__
#define _H_HOTCALL_UNTRUSTED__

#include "stdbool.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <hotcall.h>
#include "shared_memory_ctx.h"
#include "sgx_eid.h"

#ifdef BATCHING
#define ASYNC(SM_CTX, X) X || is_hotcall_in_progress(&(SM_CTX)->hcall)
#else
#define ASYNC(SM_CTX, X) false || is_hotcall_in_progress(&(SM_CTX)->hcall)
#endif

#define _HCALL(SM_CTX, UNIQUE_ID, CONFIG, ...) \
    struct parameter CAT2(HCALL_ARGS_, UNIQUE_ID)[] = { \
        __VA_ARGS__ \
    }; \
    struct hotcall_functionconfig CAT2(HCALL_CONFIG_, UNIQUE_ID) = CONFIG; \
    CAT2(HCALL_CONFIG_, UNIQUE_ID).n_params = sizeof(CAT2(HCALL_ARGS_, UNIQUE_ID))/sizeof(struct parameter);\
    (SM_CTX)->hcall.batch.queue[(SM_CTX)->hcall.batch.queue_len++] = \
        get_fcall_((SM_CTX), &CAT2(HCALL_CONFIG_, UNIQUE_ID), CAT2(HCALL_ARGS_, UNIQUE_ID));\
    if(ASYNC(SM_CTX, false) != 1) { \
        make_hotcall(&(SM_CTX)->hcall); \
    }


#define BUNDLE_END() hotcall_bundle_end(_sm_ctx)
#define BUNDLE_BEGIN() hotcall_bundle_begin(_sm_ctx)

#define CHAIN_BEGIN() hotcall_bundle_chain_begin(_sm_ctx)
#define CHAIN_CLOSE() hotcall_bundle_chain_close(_sm_ctx)


#define HCALL(CONFIG, ...) \
    _HCALL(_sm_ctx, UNIQUE_ID, CONFIG, __VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

bool
hotcall_test();
void
hotcall_init(struct shared_memory_ctx *sm_ctx, sgx_enclave_id_t _global_eid);
void
hotcall_bundle_flush(struct shared_memory_ctx *sm_ctx);
void
hotcall_bundle_expected_value(struct preallocated_function_calls *pfc, int expected, int error_code, bool has_else);
void
hotcall_destroy(struct shared_memory_ctx *sm_ctx);
struct shared_memory_ctx *
get_context();

extern inline bool
is_hotcall_in_progress(struct hotcall *hcall) {
    return hcall->hotcall_in_progress;
}

extern inline void
hotcall_bundle_begin(struct shared_memory_ctx *sm_ctx) {
    sm_ctx->hcall.batch.error = 0;
    sm_ctx->hcall.hotcall_in_progress = true;
}

extern inline void
hotcall_bundle_end(struct shared_memory_ctx *sm_ctx) {
    hotcall_bundle_flush(sm_ctx);
    sm_ctx->hcall.hotcall_in_progress = false;
}

extern inline int
hotcall_bundle_get_error() {
    return _sm_ctx->hcall.batch.error;
}

extern inline void
hotcall_bundle_chain_begin(struct shared_memory_ctx *sm_ctx) {
    sm_ctx->hcall.is_inside_chain = true;
}

extern inline void
hotcall_bundle_chain_close(struct shared_memory_ctx *sm_ctx) {
    sm_ctx->hcall.is_inside_chain = false;
}

extern inline bool
is_inside_chain(struct shared_memory_ctx *sm_ctx) {
    return sm_ctx->hcall.is_inside_chain;
}

extern inline struct ecall_queue_item *
next_queue_item(struct shared_memory_ctx *sm_ctx) {
    struct ecall_queue_item *item;
    item = &sm_ctx->pfc.fcs[sm_ctx->pfc.idx++];
    if(sm_ctx->pfc.idx == MAX_TS) {
        sm_ctx->pfc.idx = 0;
    }
    return item;
}

extern inline void *
enqueue_item(struct shared_memory_ctx *sm_ctx, struct ecall_queue_item *item) {
    sm_ctx->hcall.batch.queue[sm_ctx->hcall.batch.queue_len++] = item;
}

extern inline
struct ecall_queue_item * get_fcall_(struct shared_memory_ctx *sm_ctx, struct hotcall_functionconfig *config, struct parameter *params) {
    struct ecall_queue_item *item;
    item = next_queue_item(sm_ctx);
    item->type = QUEUE_ITEM_TYPE_FUNCTION;

    struct hotcall_function *fcall;
    fcall = &item->call.fc;
    //fcall->function_id = f_id;
    fcall->config = config;
    fcall->params = params;
    if(config->has_return) {
        fcall->return_value = fcall->params[--fcall->config->n_params].value.variable.arg;
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

extern inline void
hotcall_bundle_if_(struct shared_memory_ctx *sm_ctx, struct if_config *config, struct parameter *params) {
    struct ecall_queue_item *item;
    item = next_queue_item(sm_ctx);
    item->type = QUEUE_ITEM_TYPE_IF;
    struct hotcall_if *tif;
    tif = &item->call.tif;
    tif->params = params;
    tif->config = config;
    enqueue_item(sm_ctx, item);
}

extern inline void
hotcall_bundle_if_then(struct shared_memory_ctx *sm_ctx) {

}

extern inline void
hotcall_bundle_if_else(struct shared_memory_ctx *sm_ctx) {
    struct ecall_queue_item *item;
    item = next_queue_item(sm_ctx);
    item->type = QUEUE_ITEM_TYPE_IF_ELSE;
    enqueue_item(sm_ctx, item);
}

extern inline void
hotcall_bundle_if_end(struct shared_memory_ctx *sm_ctx) {
    struct hotcall_batch *batch;
    struct ecall_queue_item *it;
    batch = &sm_ctx->hcall.batch;
    unsigned int branch_len = 0, else_len = 0, then_len = 0;
    for(it = batch->queue[batch->queue_len - 1]; it-> type != QUEUE_ITEM_TYPE_IF; --it) {
        if(it->type == QUEUE_ITEM_TYPE_IF_ELSE) {
            else_len = branch_len;
            branch_len = 1;
        } else {
            branch_len++;
        }
    }
    it->call.tif.config->else_branch_len = else_len;
    it->call.tif.config->then_branch_len = branch_len;
}



extern inline void
chain_operators(struct shared_memory_ctx *sm_ctx, struct parameter *params) {
    struct ecall_queue_item *prev_item;
    prev_item = sm_ctx->hcall.batch.queue[sm_ctx->hcall.batch.queue_len - 1];
    switch(prev_item->type) {
        case QUEUE_ITEM_TYPE_FILTER:
            params[0] = prev_item->call.fi.params[prev_item->call.fi.config->n_params - 1];
            break;
        case QUEUE_ITEM_TYPE_MAP:
            params[0] = prev_item->call.ma.params[prev_item->call.ma.config->n_params - 1];
            break;
        case QUEUE_ITEM_TYPE_REDUCE:
            params[0] = prev_item->call.ma.params[prev_item->call.re.config->n_params - 1];
            break;
        default:
            printf("default chaining..\n");
    }
}

extern inline void
hotcall_bundle_filter(struct shared_memory_ctx *sm_ctx, struct filter_config *config, struct parameter *params) {
    struct ecall_queue_item *item;
    item = next_queue_item(sm_ctx);
    item->type = QUEUE_ITEM_TYPE_FILTER;
    struct hotcall_filter *fi;
    fi = &item->call.fi;
    fi->config = config;
    fi->params = params;
    if(is_inside_chain(sm_ctx) && sm_ctx->hcall.batch.queue_len > 0) {
        chain_operators(sm_ctx, params);
    }
    enqueue_item(sm_ctx, item);
}

extern inline void
hotcall_bundle_map(struct shared_memory_ctx *sm_ctx, struct map_config *config, struct parameter *params) {
    struct ecall_queue_item *item;
    item = next_queue_item(sm_ctx);
    item->type = QUEUE_ITEM_TYPE_MAP;
    struct hotcall_map *ma;
    ma = &item->call.ma;
    ma->config = config;
    ma->params = params;
    if(is_inside_chain(sm_ctx) && sm_ctx->hcall.batch.queue_len > 0) {
        chain_operators(sm_ctx, params);
    }
    enqueue_item(sm_ctx, item);
}

extern inline void
hotcall_bundle_reduce(struct shared_memory_ctx *sm_ctx, struct reduce_config *config, struct parameter *params) {
    struct ecall_queue_item *item;
    item = next_queue_item(sm_ctx);
    item->type = QUEUE_ITEM_TYPE_REDUCE;
    struct hotcall_reduce *re;
    re = &item->call.re;
    re->config = config;
    re->params = params;
    if(is_inside_chain(sm_ctx) && sm_ctx->hcall.batch.queue_len > 0) {
        chain_operators(sm_ctx, params);
    }
    enqueue_item(sm_ctx, item);
}

extern inline void
hotcall_bundle_do_while(struct shared_memory_ctx *sm_ctx, struct do_while_config *config, struct parameter *body_params, struct parameter *condition_params) {
    struct ecall_queue_item *item;
    item = next_queue_item(sm_ctx);
    item->type = QUEUE_ITEM_TYPE_DO_WHILE;
    struct hotcall_do_while *dw;
    dw = &item->call.dw;
    dw->config = config;
    dw->body_params = body_params;
    dw->condition_params = condition_params;
    enqueue_item(sm_ctx, item);
}

extern inline void
hotcall_bundle_for_each(struct shared_memory_ctx *sm_ctx, struct for_each_config *config, struct parameter *params) {
    struct ecall_queue_item *item;
    item = next_queue_item(sm_ctx);
    item->type = QUEUE_ITEM_TYPE_FOR_EACH;
    struct hotcall_for_each *tor;
    tor = &item->call.tor;
    tor->params = params;
    tor->config = config;
    enqueue_item(sm_ctx, item);
}

extern inline void
hotcall_bundle_for_begin(struct shared_memory_ctx *sm_ctx, struct for_config *config) {
    struct ecall_queue_item *item;
    item = next_queue_item(sm_ctx);
    item->type = QUEUE_ITEM_TYPE_FOR_BEGIN;
    struct hotcall_for_start *for_s;
    for_s = &item->call.for_s;
    for_s->config = config;
    for_s->config->loop_in_process = false;
    enqueue_item(sm_ctx, item);
}

extern inline void
hotcall_bundle_while_begin(struct shared_memory_ctx *sm_ctx, struct while_config *config, struct parameter *params) {
    struct ecall_queue_item *item;
    item = next_queue_item(sm_ctx);
    item->type = QUEUE_ITEM_TYPE_WHILE_BEGIN;
    struct hotcall_while_start *while_s;
    while_s = &item->call.while_s;
    while_s->config = config;
    while_s->params = params;
    while_s->config->loop_in_process = false;
    enqueue_item(sm_ctx, item);
}


extern inline void
calculate_loop_length(struct hotcall_batch *batch, int type) {
    unsigned body_len = 0, nesting = 0;
    struct ecall_queue_item *it;
    for(it = batch->queue[batch->queue_len - 1]; it->type != type || nesting > 0 ; --it) {
        switch(it->type) {
            case QUEUE_ITEM_TYPE_LOOP_END: nesting++; break;
            case QUEUE_ITEM_TYPE_WHILE_BEGIN: case QUEUE_ITEM_TYPE_FOR_BEGIN: nesting--; break;
        }
        body_len++;
    }
    switch(type) {
        case QUEUE_ITEM_TYPE_FOR_BEGIN:
            it->call.for_s.config->body_len = body_len;
            break;
        case QUEUE_ITEM_TYPE_WHILE_BEGIN:
            it->call.while_s.config->body_len = body_len;
            break;
        default:
            SWITCH_DEFAULT_REACHED
    }
}

extern inline void
hotcall_bundle_for_end(struct shared_memory_ctx *sm_ctx) {
    struct ecall_queue_item *item;
    item = next_queue_item(sm_ctx);
    item->type = QUEUE_ITEM_TYPE_LOOP_END;
    calculate_loop_length(&sm_ctx->hcall.batch, QUEUE_ITEM_TYPE_FOR_BEGIN);
    enqueue_item(sm_ctx, item);
}

extern inline void
hotcall_bundle_while_end(struct shared_memory_ctx *sm_ctx) {
    struct ecall_queue_item *item;
    item = next_queue_item(sm_ctx);
    item->type = QUEUE_ITEM_TYPE_LOOP_END;
    calculate_loop_length(&sm_ctx->hcall.batch, QUEUE_ITEM_TYPE_WHILE_BEGIN);
    enqueue_item(sm_ctx, item);
}

extern inline void
hotcall_bundle_error(struct shared_memory_ctx *sm_ctx, int error_code) {
    struct ecall_queue_item *item;
    item = next_queue_item(sm_ctx);
    item->type = QUEUE_ITEM_TYPE_ERROR;
    struct hotcall_error *error;
    error = &item->call.err;
    error->error_code = error_code;
    enqueue_item(sm_ctx, item);
}

#ifdef __cplusplus
}
#endif

#endif
