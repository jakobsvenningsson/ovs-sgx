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
#define ASYNC(X) X
#define _ASYNC(SM_CTX, X) X || is_hotcall_in_progress(&(SM_CTX)->hcall)
#else
#define ASYNC(X) false
#define _ASYNC(SM_CTX, X) false || is_hotcall_in_progress(&(SM_CTX)->hcall)
#endif

#define _HCALL(SM_CTX, UNIQUE_ID, CONFIG, ...) \
    struct parameter CAT2(HCALL_ARGS_, UNIQUE_ID)[] = { \
        __VA_ARGS__ \
    }; \
    struct hotcall_functionconfig CAT2(HCALL_CONFIG_, UNIQUE_ID) = CONFIG; \
    CAT2(HCALL_CONFIG_, UNIQUE_ID).n_params = sizeof(CAT2(HCALL_ARGS_, UNIQUE_ID))/sizeof(struct parameter);\
    (SM_CTX)->hcall.batch.queue[(SM_CTX)->hcall.batch.queue_len++] = \
        get_fcall_((SM_CTX), &CAT2(HCALL_CONFIG_, UNIQUE_ID), CAT2(HCALL_ARGS_, UNIQUE_ID));\
    if(_ASYNC(SM_CTX, false) != 1) { \
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

void
hotcall_init(struct shared_memory_ctx *sm_ctx, sgx_enclave_id_t _global_eid);
void
hotcall_destroy(struct shared_memory_ctx *sm_ctx);
void
hotcall_bundle_if(struct shared_memory_ctx *sm_ctx, struct if_config *config, struct parameter *params);
void
hotcall_bundle_if_else(struct shared_memory_ctx *sm_ctx);
void
hotcall_bundle_if_end(struct shared_memory_ctx *sm_ctx);
void
hotcall_bundle_assign_var(struct shared_memory_ctx *sm_ctx, struct parameter *dst, struct parameter *src);
void
hotcall_bundle_assign_ptr(struct shared_memory_ctx *sm_ctx, struct parameter *dst, struct parameter *src);
void
hotcall_bundle_filter(struct shared_memory_ctx *sm_ctx, struct filter_config *config, struct parameter *params);
void
hotcall_bundle_map(struct shared_memory_ctx *sm_ctx, struct map_config *config, struct parameter *params);
void
hotcall_bundle_reduce(struct shared_memory_ctx *sm_ctx, struct reduce_config *config, struct parameter *params);
void
hotcall_bundle_do_while(struct shared_memory_ctx *sm_ctx, struct do_while_config *config, struct parameter *body_params, struct parameter *condition_params);
void
hotcall_bundle_for_each(struct shared_memory_ctx *sm_ctx, struct for_each_config *config, struct parameter *params);
void
hotcall_bundle_for_begin(struct shared_memory_ctx *sm_ctx, struct for_config *config);
void
hotcall_bundle_while_begin(struct shared_memory_ctx *sm_ctx, struct while_config *config, struct parameter *params);
void
hotcall_bundle_for_end(struct shared_memory_ctx *sm_ctx);
void
hotcall_bundle_while_end(struct shared_memory_ctx *sm_ctx);
void
hotcall_bundle_error(struct shared_memory_ctx *sm_ctx, int error_code);

static inline void
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

static inline void
hotcall_bundle_flush(struct shared_memory_ctx *sm_ctx) {
    make_hotcall(&sm_ctx->hcall);
}

static inline bool
is_hotcall_in_progress(struct hotcall *hcall) {
    return hcall->hotcall_in_progress;
}

static inline void
hotcall_bundle_begin(struct shared_memory_ctx *sm_ctx) {
    sm_ctx->hcall.batch.error = 0;
    sm_ctx->hcall.hotcall_in_progress = true;
}

static inline void
hotcall_bundle_end(struct shared_memory_ctx *sm_ctx) {
    hotcall_bundle_flush(sm_ctx);
    sm_ctx->hcall.hotcall_in_progress = false;
}

static inline int
hotcall_bundle_get_error() {
    return _sm_ctx->hcall.batch.error;
}

static inline void
hotcall_bundle_chain_begin(struct shared_memory_ctx *sm_ctx) {
    sm_ctx->hcall.is_inside_chain = true;
}

static inline void
hotcall_bundle_chain_close(struct shared_memory_ctx *sm_ctx) {
    sm_ctx->hcall.is_inside_chain = false;
}

static inline bool
is_inside_chain(struct shared_memory_ctx *sm_ctx) {
    return sm_ctx->hcall.is_inside_chain;
}

static inline struct ecall_queue_item *
next_queue_item(struct shared_memory_ctx *sm_ctx) {
    struct ecall_queue_item *item;
    item = &sm_ctx->hcall.fcs[sm_ctx->hcall.idx++];
    if(sm_ctx->hcall.idx == MAX_TS) {
        sm_ctx->hcall.idx = 0;
    }
    return item;
}

static inline void *
enqueue_item(struct shared_memory_ctx *sm_ctx, struct ecall_queue_item *item) {
    sm_ctx->hcall.batch.queue[sm_ctx->hcall.batch.queue_len++] = item;
}

static inline void
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


static inline
struct ecall_queue_item * get_fcall_(struct shared_memory_ctx *sm_ctx, struct hotcall_functionconfig *config, struct parameter *params) {
    struct ecall_queue_item *item;
    item = next_queue_item(sm_ctx);
    item->type = QUEUE_ITEM_TYPE_FUNCTION;
    struct hotcall_function *fcall;
    fcall = &item->call.fc;
    fcall->config = config;
    fcall->params = params;
    if(config->has_return) {
        fcall->return_value = fcall->params[--fcall->config->n_params].value.variable.arg;
    }
    return item;
}


static inline void
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

#ifdef __cplusplus
}
#endif

#endif
