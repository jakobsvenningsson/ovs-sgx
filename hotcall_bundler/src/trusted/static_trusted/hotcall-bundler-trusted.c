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

void
hotcall_register_config(struct hotcall_config *config) {
    hotcall_config = config;
}

static inline int
hotcall_execute_bundle(struct hotcall_batch *batch) {

    struct ecall_queue_item *queue_item;
    struct hotcall_function *fc;

    uint8_t exclude_list[batch->queue_len];
    memset(exclude_list, 0, batch->queue_len);
    struct loop_stack_item loop_stack[MAX_LOOP_RECURSION] = { 0 };
    unsigned int loop_stack_pos = 0, queue_length = batch->queue_len;

    int n = 0;
    while(n < queue_length) {
        queue_item = batch->queue[n];
        if(exclude_list[n]) {
            n++;
            continue;
        }

        switch(queue_item->type) {
            case QUEUE_ITEM_TYPE_DESTROY:
                return -1;
            case QUEUE_ITEM_TYPE_FUNCTION:
                fc = &queue_item->call.fc;
                parse_function_arguments(fc->params, fc->config->n_params, (loop_stack_pos > 0 ? loop_stack[loop_stack_pos - 1].offset : 0), fc->args);
                hotcall_config->execute_function(fc->config->function_id, (void **) fc->args, fc->return_value);
                break;
            case QUEUE_ITEM_TYPE_IF: case QUEUE_ITEM_TYPE_IF_NULL:
                hotcall_handle_if(&queue_item->call.tif, hotcall_config, exclude_list, n, queue_length, loop_stack_pos > 0 ? loop_stack[loop_stack_pos - 1].offset : 0);
                break;
            case QUEUE_ITEM_TYPE_FOR_EACH:
                hotcall_handle_for_each(&queue_item->call.tor, hotcall_config);
                break;
            case QUEUE_ITEM_TYPE_FOR_BEGIN:
                loop_stack_pos = hotcall_handle_for_begin(&queue_item->call.for_s, loop_stack, loop_stack_pos, exclude_list, n);
                break;
            case QUEUE_ITEM_TYPE_FILTER:
                hotcall_handle_filter(&queue_item->call.fi, hotcall_config);
                break;
            case QUEUE_ITEM_TYPE_DO_WHILE:
                hotcall_handle_do_while(&queue_item->call.dw, hotcall_config);
                break;
            case QUEUE_ITEM_TYPE_FOR_END:
                loop_stack_pos = hotcall_handle_for_end(loop_stack, loop_stack_pos, &n, exclude_list);
                break;
            case QUEUE_ITEM_TYPE_WHILE_BEGIN:
                loop_stack_pos = hotcall_handle_while_begin(&queue_item->call.while_s, hotcall_config, loop_stack, loop_stack_pos, n, exclude_list);
                break;
            case QUEUE_ITEM_TYPE_WHILE_END:
                n = n - (loop_stack[loop_stack_pos - 1].body_len + 1);
                goto continue_loop;
            case QUEUE_ITEM_TYPE_MAP:
                hotcall_handle_map(&queue_item->call.ma, hotcall_config);
                break;
            case QUEUE_ITEM_TYPE_REDUCE:
                hotcall_handle_reduce(&queue_item->call.re, hotcall_config);
                break;
            case QUEUE_ITEM_TYPE_ERROR:
                batch->error = queue_item->call.err.config->error_code;
                goto batch_done;
            case QUEUE_ITEM_TYPE_IF_ELSE: case QUEUE_ITEM_TYPE_IF_END:
                break;
            default:
                SWITCH_DEFAULT_REACHED
        }
        n++;
        continue_loop: ;
    }

    batch_done:

    return 0;
}

int
hotcall_bundler_start(struct shared_memory_ctx *sm_ctx){

    int exit_code;

    while (1) {

        sgx_spin_lock(&sm_ctx->hcall.spinlock);

        if (sm_ctx->hcall.run) {
            sm_ctx->hcall.run = false;

            exit_code = hotcall_execute_bundle(&sm_ctx->hcall.batch);
            if(exit_code) goto exit;

            sm_ctx->hcall.is_done = true;
            sm_ctx->hcall.batch.queue_len = 0;
        }

        sgx_spin_unlock(&sm_ctx->hcall.spinlock);

        // Its recommended by intel to add pause actions inside spinlock loops in order to increase performance.
        for (int i = 0; i < 3; ++i) {
            __asm
            __volatile(
              "pause"
            );
        }

        for(int i = 0; i < hotcall_config->n_spinlock_jobs; ++i) {
            if(hotcall_config->spin_lock_task_count[i] == hotcall_config->spin_lock_task_timeouts[i]) {
                    hotcall_config->spin_lock_tasks[i](sm_ctx->custom_object_ptr[i]);
                    hotcall_config->spin_lock_task_count[i] = 0;
                    continue;
            }
            hotcall_config->spin_lock_task_count[i]++;
        }
    }


    exit:

    sm_ctx->hcall.batch.queue_len = 0;
    sgx_spin_unlock(&sm_ctx->hcall.spinlock);
    sm_ctx->hcall.is_done = true;

    return 0;
} /* ecall_start_poller */
