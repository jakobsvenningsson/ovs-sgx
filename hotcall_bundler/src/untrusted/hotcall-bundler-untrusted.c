#include "hotcall_bundler_u.h"
#include "hotcall-bundler-untrusted.h"
#include <pthread.h>
#include <assert.h>
#include "postfix_translator.h"

struct shared_memory_ctx *_sm_ctx;
static sgx_enclave_id_t global_eid;
struct hotcall_config * _hotcall_config;

void *
start_enclave_thread(void * vargp){
    printf("start_enclave_thread\n");
    int ecall_return;
    hotcall_bundler_start(global_eid, &ecall_return, _sm_ctx);
    if (ecall_return == 0) {
        printf("Application ran with success\n");
    } else {
        printf("Application failed %d \n", ecall_return);
    }
}

void
hotcall_init(struct shared_memory_ctx *ctx, sgx_enclave_id_t eid) {

    ctx->hcall.hotcall_in_progress = false;
    ctx->hcall.batch.queue_len = 0;
    ctx->hcall.is_inside_chain = false;
    ctx->hcall.batch.ignore_hcalls = false;

    global_eid = eid;
    _sm_ctx = ctx;

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, start_enclave_thread, NULL);
}

void
hotcall_destroy(struct shared_memory_ctx *sm_ctx) {
    struct ecall_queue_item *item;
    item = next_queue_item(sm_ctx);
    item->type = QUEUE_ITEM_TYPE_DESTROY;
    enqueue_item(sm_ctx, item);
    hotcall_bundle_flush(sm_ctx);
}

void
hotcall_enqueue_item(struct shared_memory_ctx *sm_ctx, uint8_t item_type, void *config, struct parameter *params) {
    struct ecall_queue_item *item;
    item = next_queue_item(sm_ctx);
    item->type = item_type;
    switch(item_type) {
        case QUEUE_ITEM_TYPE_IF:
            item->call.tif.params = params;
            item->call.tif.config = (struct if_config *) config;
            item->call.tif.config->postfix_length = to_postfix(item->call.tif.config->predicate_fmt, item->call.tif.params, item->call.tif.config->postfix);
            break;
        case QUEUE_ITEM_TYPE_IF_ELSE:
            break;
        case QUEUE_ITEM_TYPE_IF_END:
            calculate_if_body_length(sm_ctx);
            break;
        case QUEUE_ITEM_TYPE_FILTER:
            item->call.fi.config = (struct filter_config *) config;
            item->call.fi.params = params;
            item->call.fi.config->postfix_length = to_postfix(item->call.fi.config->predicate_fmt, item->call.fi.params, item->call.fi.config->postfix);
            if(is_inside_chain(sm_ctx) && sm_ctx->hcall.batch.queue_len > 0) {
                chain_operators(sm_ctx, params);
            }
            break;
        case QUEUE_ITEM_TYPE_MAP:
            item->call.ma.config = (struct map_config *) config;
            item->call.ma.params = params;
            if(is_inside_chain(sm_ctx) && sm_ctx->hcall.batch.queue_len > 0) {
                chain_operators(sm_ctx, params);
            }
            break;
        case QUEUE_ITEM_TYPE_REDUCE:
            item->call.re.config = (struct reduce_config *) config;
            item->call.re.params = params;
            if(is_inside_chain(sm_ctx) && sm_ctx->hcall.batch.queue_len > 0) {
                chain_operators(sm_ctx, params);
            }
            break;
        case QUEUE_ITEM_TYPE_DO_WHILE:
            item->call.dw.config = (struct do_while_config *) config;
            item->call.dw.body_params = params;
            item->call.dw.config->postfix_length = to_postfix(item->call.dw.config->predicate_fmt, item->call.dw.config->condition_params, item->call.dw.config->postfix);
            break;
        case QUEUE_ITEM_TYPE_FOR_EACH:
            item->call.tor.params = params;
            item->call.tor.config = (struct for_each_config *) config;
            break;
        case QUEUE_ITEM_TYPE_FOR_BEGIN:
            item->call.for_s.config = (struct for_config *) config;
            item->call.for_s.config->loop_in_process = false;
            break;
        case QUEUE_ITEM_TYPE_FOR_END:
            calculate_loop_length(&sm_ctx->hcall, QUEUE_ITEM_TYPE_FOR_BEGIN);
            break;
        case QUEUE_ITEM_TYPE_WHILE_BEGIN:
            item->call.while_s.config = (struct while_config *) config;
            item->call.while_s.params = params;
            item->call.while_s.config->loop_in_process = false;
            item->call.while_s.config->postfix_length = to_postfix(item->call.while_s.config->predicate_fmt, item->call.while_s.params, item->call.while_s.config->postfix);
            break;
        case QUEUE_ITEM_TYPE_WHILE_END:
            calculate_loop_length(&sm_ctx->hcall, QUEUE_ITEM_TYPE_WHILE_BEGIN);
            break;
        case QUEUE_ITEM_TYPE_ERROR:
            item->call.err.config = (struct error_config *) config;
            break;
        default: SWITCH_DEFAULT_REACHED
    }
    enqueue_item(sm_ctx, item);
}
