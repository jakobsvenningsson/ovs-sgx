#include "hotcall_bundler_t.h"  /* print_string */
#include "hotcall-bundler-trusted.h"
#include "if.h"
#include "do_while.h"
#include "reduce.h"
#include "map.h"
#include "for_each.h"
#include "for.h"
#include "while.h"
#include "parameter.h"
#include "filter.h"
#include "execute_function.h"
#include "assert.h"


void
hotcall_register_config(struct hotcall_config *config) {
    hotcall_config = config;
}

static inline void
hotcall_handle_function(struct ecall_queue_item *qi, const struct hotcall_config *hotcall_config, struct queue_context *queue_ctx, struct batch_status * batch_status) {
    struct hotcall_function *fc = &qi->call.fc;
    unsigned int loop_stack_pos = queue_ctx->loop_stack_pos;
    parse_function_arguments(fc->params, fc->config->n_params, loop_stack_pos > 0 ? queue_ctx->loop_stack[loop_stack_pos - 1].offset : 0, fc->args);
    execute_function(hotcall_config, fc->config->function_id, 1, fc->config->n_params, fc->args);
}

static inline void
hotcall_handle_error(struct ecall_queue_item *qi, const struct hotcall_config *hotcall_config, struct queue_context *queue_ctx, struct batch_status * batch_status) {
    batch_status->exit_batch = true;
    batch_status->error = qi->call.err.config->error_code;
}

static inline void
hotcall_handle_destroy(struct ecall_queue_item *qi, const struct hotcall_config *hotcall_config, struct queue_context *queue_ctx, struct batch_status * batch_status) {
    batch_status->exit_batch = true;
    batch_status->error = -1;
}


static void *lookup_table[256] = {
    [QUEUE_ITEM_TYPE_DESTROY] = hotcall_handle_destroy,
    [QUEUE_ITEM_TYPE_FUNCTION] = hotcall_handle_function,
    [QUEUE_ITEM_TYPE_FOR_BEGIN] = hotcall_handle_for_begin,
    [QUEUE_ITEM_TYPE_FOR_EACH] = hotcall_handle_for_each,
    [QUEUE_ITEM_TYPE_FILTER] = hotcall_handle_filter,
    [QUEUE_ITEM_TYPE_DO_WHILE] = hotcall_handle_do_while,
    [QUEUE_ITEM_TYPE_FOR_END] = hotcall_handle_for_end,
    [QUEUE_ITEM_TYPE_WHILE_BEGIN] = hotcall_handle_while_begin,
    [QUEUE_ITEM_TYPE_WHILE_END] = hotcall_handle_while_end,
    [QUEUE_ITEM_TYPE_MAP] = hotcall_handle_map,
    [QUEUE_ITEM_TYPE_REDUCE] = hotcall_handle_reduce,
    [QUEUE_ITEM_TYPE_ERROR] = hotcall_handle_error,
    [QUEUE_ITEM_TYPE_IF] = hotcall_handle_if,
    [QUEUE_ITEM_TYPE_IF_ELSE] = hotcall_handle_if_else,
    [QUEUE_ITEM_TYPE_IF_END] = NULL,
    [QUEUE_ITEM_TYPE_ASSERT] = hotcall_handle_assert
};


static inline void
execute_tasks(struct shared_memory_ctx *sm_ctx, struct hotcall_config *hotcall_config) {
    for(int i = 0; i < hotcall_config->n_spinlock_jobs; ++i) {
        if(hotcall_config->spin_lock_task_count[i] == hotcall_config->spin_lock_task_timeouts[i]) {
                hotcall_config->spin_lock_tasks[i](sm_ctx->custom_object_ptr[i]);
                hotcall_config->spin_lock_task_count[i] = 0;
                continue;
        }
        hotcall_config->spin_lock_task_count[i]++;
    }
}

static inline int
hotcall_execute_bundle(struct hotcall_batch *batch) {

    register struct ecall_queue_item *queue_item;
    register void (*f)(struct ecall_queue_item *, struct hotcall_config *, struct queue_context *, struct batch_status *);

    unsigned int queue_len = batch->queue_len;
    struct queue_context queue_ctx = { .len = queue_len };
    struct batch_status status = { 0 };


    register struct ecall_queue_item *item = batch->queue;
    while(item) {
        f = lookup_table[item->type];
        if(!f) { item = item->next; continue; };
        f(item, hotcall_config, &queue_ctx, &status);
        if(status.exit_batch) {
            if(status.error == -1) return -1;
            batch->error = status.error;
            goto batch_done;
        };
        item = item->next;
    }

    batch_done:

    return 0;
}

static inline int
hotcall_execute_ecall(struct ecall_queue_item *qi) {
    if(qi->type == QUEUE_ITEM_TYPE_DESTROY) {
        return -1;
    }
    struct hotcall_function *fc = &qi->call.fc;
    parse_function_arguments(fc->params, fc->config->n_params, 0, fc->args);
    execute_function(hotcall_config, fc->config->function_id, 1, fc->config->n_params, fc->args);
    return 0;
}

int
hotcall_bundler_start(struct shared_memory_ctx *sm_ctx){

    printf("Hotcall thread started..\n");

    int exit_code;

    while (1) {

        __asm
        __volatile(
          "pause"
        );

        sgx_spin_lock(&sm_ctx->hcall.spinlock);

        if (sm_ctx->hcall.run) {
            sm_ctx->hcall.run = false;
            if(sm_ctx->hcall.batch) exit_code = hotcall_execute_bundle(sm_ctx->hcall.batch);
            else exit_code = hotcall_execute_ecall(sm_ctx->hcall.ecall);
            sm_ctx->hcall.is_done = true;
            if(exit_code) goto exit;
        }

        sgx_spin_unlock(&sm_ctx->hcall.spinlock);

        // Inform CPU that we are inside a spinlock loop
        __asm
        __volatile(
          "pause"
        );

        /*sif(hotcall_config->n_spinlock_jobs > 0) {
            execute_tasks(sm_ctx, hotcall_config);
        }*/
    }


    exit:

    sgx_spin_unlock(&sm_ctx->hcall.spinlock);

    return 0;
} /* ecall_start_poller */
