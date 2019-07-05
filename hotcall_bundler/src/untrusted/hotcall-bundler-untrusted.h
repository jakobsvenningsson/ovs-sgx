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

#define BUNDLE_END() \
    hotcall_bundle_flush(_sm_ctx);\
    _sm_ctx->hcall.hotcall_in_progress = false;\
    _sm_ctx->hcall.error = _sm_ctx->hcall.batch->error; \
    _sm_ctx->hcall.batch = NULL

#define _BUNDLE_BEGIN(ID) \
    struct hotcall_batch ID = { 0 }; \
    _sm_ctx->hcall.batch = &ID;\
    _sm_ctx->hcall.hotcall_in_progress = true;\
    _sm_ctx->hcall.error = 0

#define BUNDLE_BEGIN() _BUNDLE_BEGIN(UNIQUE_ID)

#define CHAIN_BEGIN() hotcall_bundle_chain_begin(_sm_ctx)
#define CHAIN_CLOSE() hotcall_bundle_chain_close(_sm_ctx)

#define BUNDLE_IF_TRUE(CONDITION) \
    _sm_ctx->hcall.batch->ignore_hcalls = CONDITION ? false : true;
#define BUNDLE_IF_ELSE() \
    _sm_ctx->hcall.batch->ignore_hcalls = !_sm_ctx->hcall.batch->ignore_hcalls;
#define BUNDLE_IF_END() \
    _sm_ctx->hcall.batch->ignore_hcalls = false;


#ifdef __cplusplus
extern "C" {
#endif

void
hotcall_enqueue_item(struct shared_memory_ctx *sm_ctx, uint8_t item_type, void *config, struct parameter *params, struct ecall_queue_item *item);
void
hotcall_init(struct shared_memory_ctx *sm_ctx, sgx_enclave_id_t _global_eid);
void
hotcall_destroy(struct shared_memory_ctx *sm_ctx);

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
hotcall_bundle_begin(struct shared_memory_ctx *sm_ctx, struct hotcall_batch *batch) {
    sm_ctx->hcall.batch = batch;
    sm_ctx->hcall.hotcall_in_progress = true;
}

static inline void
hotcall_bundle_end(struct shared_memory_ctx *sm_ctx) {
    hotcall_bundle_flush(sm_ctx);
    sm_ctx->hcall.hotcall_in_progress = false;
    sm_ctx->hcall.batch = NULL;
}

static inline int
hotcall_bundle_get_error() {
    return _sm_ctx->hcall.error;
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

static inline void
calculate_loop_length(struct hotcall *hcall, int type) {
    struct hotcall_batch *batch;
    batch = hcall->batch;
    unsigned nesting = 0;
    struct ecall_queue_item *it;
    for(it = batch->top->prev; it->type != type || nesting > 0 ; it = it->prev) {
        switch(it->type) {
            case QUEUE_ITEM_TYPE_WHILE_END: case QUEUE_ITEM_TYPE_FOR_END: nesting++; break;
            case QUEUE_ITEM_TYPE_WHILE_BEGIN: case QUEUE_ITEM_TYPE_FOR_BEGIN: nesting--; break;
        }
    }
    switch(type) {
        case QUEUE_ITEM_TYPE_FOR_BEGIN: it->call.for_s.config->loop_end = batch->top; break;
        case QUEUE_ITEM_TYPE_WHILE_BEGIN:
            it->call.while_s.config->loop_start = it->next;
            it->call.while_s.config->loop_end = batch->top;
            break;
        default: SWITCH_DEFAULT_REACHED
    }
}

static inline void
calculate_if_body_length(struct shared_memory_ctx *sm_ctx) {
    struct hotcall_batch *batch;
    struct ecall_queue_item *it;
    batch = sm_ctx->hcall.batch;
    unsigned int nesting = 0;
    struct ecall_queue_item *else_branch = NULL;

    for(it = batch->top->prev; it->type != QUEUE_ITEM_TYPE_IF || nesting > 0; it = it->prev) {
        if(it->type == QUEUE_ITEM_TYPE_IF_END) nesting++;
        else if(it->type == QUEUE_ITEM_TYPE_IF) nesting --;

        if(it->type == QUEUE_ITEM_TYPE_IF_ELSE && nesting == 0) {
            else_branch = it->next;
            it->call.tife.config->if_end = batch->top;
        }
    }
    it->call.tif.config->else_branch = else_branch;
    it->call.tif.config->then_branch = it->next;
    it->call.tif.config->if_end = batch->top;
}


static inline
struct ecall_queue_item * get_fcall_(struct shared_memory_ctx *sm_ctx, struct hotcall_function_config *config, struct parameter *params, struct ecall_queue_item *item) {
    item->type = QUEUE_ITEM_TYPE_FUNCTION;
    struct hotcall_function *fcall;
    fcall = &item->call.fc;
    fcall->config = config;
    fcall->params = params;

    if(!sm_ctx->hcall.batch->queue) {
        sm_ctx->hcall.batch->queue = sm_ctx->hcall.batch->top = item;
    } else {
        item->prev = sm_ctx->hcall.batch->top;
        sm_ctx->hcall.batch->top->next = item;
        sm_ctx->hcall.batch->top = item;
    }
}


static inline void
chain_operators(struct shared_memory_ctx *sm_ctx, struct parameter *params) {
    struct ecall_queue_item *prev_item;
    prev_item = sm_ctx->hcall.batch->top->prev;
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
