#include "hotcall_bundler_u.h"
#include "hotcall-bundler-untrusted.h"
#include <pthread.h>
#include <assert.h>

struct shared_memory_ctx *_sm_ctx;
static sgx_enclave_id_t global_eid;

void *
start_enclave_thread(void * vargp){
    printf("start_enclave_thread\n");
    int ecall_return;
    ecall_start_poller(global_eid, &ecall_return, _sm_ctx);
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

static inline
int presedence(char c) {
    switch(c) {
        case '!':
            return 4;
        case '>': case '<': case '%':
            return 3;
        case '&': case '|':
            return 2;
        default:
            return 0;
    }
}



unsigned int
to_postfix(const char *condition_fmt, struct parameter *predicate_args, struct postfix_item *output) {
    char chs[128];
    unsigned int pos = 0, output_index = 0, variable_index = 0;
    char tmp;
    const char *input = condition_fmt;
    for(int i = 0; i < strlen(input); ++i) {

        if(input[i] >= 'a' && input[i] <= 'z') {
            output[output_index++] = (struct postfix_item) { input[i], &predicate_args[variable_index++] };
            continue;
        }

        switch(input[i]) {
            case '(': chs[pos++] = input[i]; break;
            case ')':
                while(pos > 0 && (tmp = chs[pos - 1]) != '(') {
                    output[output_index++] = (struct postfix_item) { tmp };
                    pos--;
                }
                if(pos > 0 && tmp == '(') {
                    pos--;
                }
                break;
            default:
                while(pos > 0 && chs[pos - 1] != '(' && presedence(input[i]) <= presedence(chs[pos - 1])) {
                    tmp = chs[--pos];
                    output[output_index++] = (struct postfix_item) { tmp };
                }
                chs[pos++] = input[i];
        }
    }

    while(pos > 0) {
        tmp = chs[--pos];
        output[output_index++].ch = tmp;
    }
    return output_index;
}


void
hotcall_bundle_if(struct shared_memory_ctx *sm_ctx, struct if_config *config, struct parameter *params, struct postfix_item *postfix) {
    struct ecall_queue_item *item;
    item = next_queue_item(sm_ctx);
    item->type = QUEUE_ITEM_TYPE_IF;
    struct hotcall_if *tif;
    tif = &item->call.tif;
    tif->params = params;
    tif->config = config;
    tif->config->postfix = postfix;
    tif->config->postfix_length = to_postfix(tif->config->predicate_fmt, tif->params, tif->config->postfix);
    enqueue_item(sm_ctx, item);
}

void
hotcall_bundle_if_else(struct shared_memory_ctx *sm_ctx) {
    struct ecall_queue_item *item;
    item = next_queue_item(sm_ctx);
    item->type = QUEUE_ITEM_TYPE_IF_ELSE;
    enqueue_item(sm_ctx, item);
}

void
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
        if(it == sm_ctx->hcall.fcs) {
            it = sm_ctx->hcall.fcs + MAX_FCS;
        }
    }
    it->call.tif.config->else_branch_len = else_len;
    it->call.tif.config->then_branch_len = branch_len;
}



void
hotcall_bundle_assign_var(struct shared_memory_ctx *sm_ctx, struct parameter *dst, struct parameter *src) {
    struct ecall_queue_item *item;
    item = next_queue_item(sm_ctx);
    item->type = QUEUE_ITEM_TYPE_ASSIGN_VAR;
    item->call.var.src = src;
    item->call.var.dst = dst;
    item->call.var.offset = 0;
    enqueue_item(sm_ctx, item);
}

void
hotcall_bundle_assign_ptr(struct shared_memory_ctx *sm_ctx, struct parameter *dst, struct parameter *src) {
    struct ecall_queue_item *item;
    item = next_queue_item(sm_ctx);
    item->type = QUEUE_ITEM_TYPE_ASSIGN_PTR;
    item->call.ptr.src = src;
    item->call.ptr.dst = dst;
    item->call.ptr.offset = 0;
    enqueue_item(sm_ctx, item);
}

void
hotcall_bundle_filter(struct shared_memory_ctx *sm_ctx, struct filter_config *config, struct parameter *params, struct postfix_item *postfix) {
    struct ecall_queue_item *item;
    item = next_queue_item(sm_ctx);
    item->type = QUEUE_ITEM_TYPE_FILTER;
    struct hotcall_filter *fi;
    fi = &item->call.fi;
    fi->config = config;
    fi->params = params;
    fi->config->postfix = postfix;
    fi->config->postfix_length = to_postfix(fi->config->predicate_fmt, fi->params, fi->config->postfix);
    if(is_inside_chain(sm_ctx) && sm_ctx->hcall.batch.queue_len > 0) {
        chain_operators(sm_ctx, params);
    }
    enqueue_item(sm_ctx, item);
}

void
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

void
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

void
hotcall_bundle_do_while(struct shared_memory_ctx *sm_ctx, struct do_while_config *config, struct parameter *body_params, struct parameter *condition_params, struct postfix_item *postfix) {
    struct ecall_queue_item *item;
    item = next_queue_item(sm_ctx);
    item->type = QUEUE_ITEM_TYPE_DO_WHILE;
    struct hotcall_do_while *dw;
    dw = &item->call.dw;
    dw->config = config;
    dw->body_params = body_params;
    dw->condition_params = condition_params;
    dw->config->postfix = postfix;
    dw->config->postfix_length = to_postfix(dw->config->predicate_fmt, dw->condition_params, dw->config->postfix);
    enqueue_item(sm_ctx, item);
}

void
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

void
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

void
hotcall_bundle_while_begin(struct shared_memory_ctx *sm_ctx, struct while_config *config, struct parameter *params, struct postfix_item *postfix) {
    struct ecall_queue_item *item;
    item = next_queue_item(sm_ctx);
    item->type = QUEUE_ITEM_TYPE_WHILE_BEGIN;
    struct hotcall_while_start *while_s;
    while_s = &item->call.while_s;
    while_s->config = config;
    while_s->params = params;
    while_s->config->loop_in_process = false;
    while_s->config->postfix = postfix;
    while_s->config->postfix_length = to_postfix(while_s->config->predicate_fmt, while_s->params, while_s->config->postfix);
    enqueue_item(sm_ctx, item);
}

void
hotcall_bundle_for_end(struct shared_memory_ctx *sm_ctx) {
    struct ecall_queue_item *item;
    item = next_queue_item(sm_ctx);
    item->type = QUEUE_ITEM_TYPE_FOR_END;
    calculate_loop_length(&sm_ctx->hcall, QUEUE_ITEM_TYPE_FOR_BEGIN);
    enqueue_item(sm_ctx, item);
}

void
hotcall_bundle_while_end(struct shared_memory_ctx *sm_ctx) {
    struct ecall_queue_item *item;
    item = next_queue_item(sm_ctx);
    item->type = QUEUE_ITEM_TYPE_WHILE_END;
    calculate_loop_length(&sm_ctx->hcall, QUEUE_ITEM_TYPE_WHILE_BEGIN);
    enqueue_item(sm_ctx, item);
}

void
hotcall_bundle_error(struct shared_memory_ctx *sm_ctx, int error_code) {
    struct ecall_queue_item *item;
    item = next_queue_item(sm_ctx);
    if(item == NULL) printf("item null\n");
    item->type = QUEUE_ITEM_TYPE_ERROR;
    struct hotcall_error *error;
    error = &item->call.err;
    error->error_code = error_code;
    enqueue_item(sm_ctx, item);
}
