#include "hotcall_bundler_t.h"  /* print_string */
#include "hotcall-trusted.h"
#include <stdio.h>
#include <ctype.h>
#include "boolean_expression_translator.h"

static struct hotcall_config *hotcall_config;

void
hotcall_register_config(struct hotcall_config *config) {
    hotcall_config = config;
}

static inline void
hotcall_handle_do_while(struct transaction_do_while *dw) {
    /*struct function_call fc;
    fc.id = dw->f;
    fc.args.n_args = dw->args.params_n;

    bool outcome;
    outcome = translate_expression(dw->args.condition, 0, 0);
    while(outcome == dw->args.condition.expected) {
        for(int j = 0; j < dw->args.params_n; ++j) {
            switch(dw->fmt[j]) {
                case 'd':
                    fc.args.args[j] = (int *) dw->args.params_in[j] + i;
                    break;
                default:
                    break;
            }
        }
        hotcall_config->execute_function(&fc);

        outcome = translate_expression(dw->args.condition, 0, 0);
    }*/
}



static inline bool
hotcall_handle_if(struct transaction_if *tif, uint8_t *exclude_list, int pos, int exclude_list_len) {
    struct postfix_item output[strlen(tif->args->fmt)];
    unsigned int output_length;
    to_postfix(tif, output, &output_length);
    /*for(int i = 0; i < output_length; ++i) {
        printf("%c %d ", output[i].ch, output[i].type == OPERAND ? (output[i].var->type == VARIABLE_TYPE ? *(bool *) output[i].var->val : -2) : -1);
    }
    printf("\n");*/
    int res = evaluate_postfix(tif, output, output_length, hotcall_config);
    if(res == tif->args->expected && tif->args->else_branch_len > 0) {
        exclude_else_branch(exclude_list, pos, tif->args->then_branch_len, tif->args->else_branch_len);
    } else if(res != tif->args->expected) {
        if(tif->args->then_branch_len > 0) {
            exclude_if_branch(exclude_list, pos, tif->args->then_branch_len);
        }
        if(tif->args->return_if_false) {
            exclude_rest(exclude_list, pos, tif->args->then_branch_len ,tif->args->else_branch_len, exclude_list_len);
        }
    }
}

static inline void
hotcall_handle_filter(struct transaction_filter *fi) {
    struct function_call fc;
    fc.id = fi->f;
    fc.args.n_args = fi->n_params;
    int n_include = 0;
    for(int i = 0; i < *fi->n_iter; ++i) {
        for(int j = 0; j < fi->n_params; ++j) {
            switch(fi->fmt[j]) {
                case 'd':
                    fc.args.args[j] = (int *) fi->params_in[j] + i;
                    break;
                default:
                    break;
            }
        }
        bool include;
        fc.return_value = (void *) &include;
        hotcall_config->execute_function(&fc);
        if(include) {
            for(int j = 0; j < fi->n_params; ++j) {
                switch(fi->fmt[j]) {
                    case 'd':
                        ((int *) fi->params_out[j])[n_include] = ((int *) fi->params_in[j])[i];
                        break;
                    default:
                        break;
                }
            }
            n_include++;
        }
    }
    *fi->filtered_length = n_include;
    //memcpy(fi->params_out[0], filtered, n_include * 4);
}

static inline void
hotcall_handle_for(struct transaction_for_each *tor) {
    struct function_call fc;
    fc.id = tor->f;
    fc.args.n_args = tor->n_params;
    for(int i = 0; i < *tor->n_iter; ++i) {
        for(int j = 0; j < tor->n_params; ++j) {
            switch(tor->fmt[j]) {
                case 'd':
                    fc.args.args[j] = (int *) tor->params[j] + i;
                    break;
                default:
                    break;
            }
        }
        hotcall_config->execute_function(&fc);
    }
}

static inline void
hotcall_handle_for_begin(struct transaction_for_start *for_s, uint8_t *for_loop_nesting, uint8_t *exclude_list, int pos) {
    if(for_s->n_iters-- > 0) {
        (*for_loop_nesting)++;
        return;
    }
    (*for_loop_nesting)--;
    memset(exclude_list + pos + 1, 1, for_s->n_rows + 1);
}

static inline void
hotcall_handle_for_end(struct transaction_for_end *for_e, int *pos, uint8_t *for_loop_nesting, unsigned int *for_loop_indices) {
    *pos = *pos - (for_e->n_rows + 2);
    (*for_loop_nesting)--;
    for_loop_indices[*for_loop_nesting]++;
}

static inline void
hotcall_handle_function(struct function_call *fc, uint8_t for_loop_nesting, unsigned int *for_loop_indices) {
    void *tmp[fc->args.n_args];
    if(for_loop_nesting > 0) {
        for(int i = 0; i < fc->args.n_args; ++i) {
            tmp[i] = fc->args.args[i];
            fc->args.args[i] = ((int *) fc->args.args[i] + for_loop_indices[for_loop_nesting - 1]);
        }
    }
    hotcall_config->execute_function(fc);
    if(for_loop_nesting > 0) {
        for(int i = 0; i < fc->args.n_args; ++i) {
            fc->args.args[i] = ((int *) fc->args.args[i] + for_loop_indices[for_loop_nesting - 1]);
            fc->args.args[i] = tmp[i];
        }
    }
}


int
ecall_start_poller(struct shared_memory_ctx *sm_ctx){

    printf("Starting shared memory poller.\n");

    struct ecall_queue_item *queue_item, *queue_item_next;
    struct function_call *fc, *prev_fc;
    struct transaction_if *tif;
    struct transaction_for_each *tor;
    struct transaction_for_end *for_e;
    struct transaction_for_start *for_s;
    struct transaction_filter *fi;

    while (1) {

        sgx_spin_lock(&sm_ctx->hcall.spinlock);

        if (sm_ctx->hcall.run) {

            sm_ctx->hcall.run = false;

            uint8_t exclude_list[sm_ctx->hcall.queue_length];
            memset(exclude_list, 0, sm_ctx->hcall.queue_length);

            unsigned int for_loop_indices[3] = { 0 };
            uint8_t for_loop_nesting = 0;

            int n;
            for(n = 0; n < sm_ctx->hcall.queue_length; ++n) {
                    if(exclude_list[n]) {
                        continue;
                    }
                    queue_item = sm_ctx->hcall.ecall_queue[n];

                    switch(queue_item->type) {
                        case QUEUE_ITEM_TYPE_DESTROY:
                            goto exit;
                        case QUEUE_ITEM_TYPE_FUNCTION:
                            hotcall_handle_function(&queue_item->call.fc, for_loop_nesting, for_loop_indices);
                            prev_fc = &queue_item->call.fc;
                            break;
                        case QUEUE_ITEM_TYPE_IF: case QUEUE_ITEM_TYPE_IF_NULL:
                            hotcall_handle_if(&queue_item->call.tif, exclude_list, n, sm_ctx->hcall.queue_length);
                            break;
                        case QUEUE_ITEM_TYPE_FOR_EACH:
                            hotcall_handle_for(&queue_item->call.tor);
                            break;
                        case QUEUE_ITEM_TYPE_FOR_BEGIN:
                            hotcall_handle_for_begin(&queue_item->call.for_s, &for_loop_nesting, exclude_list, n);
                            break;
                        case QUEUE_ITEM_TYPE_FOR_END:
                            hotcall_handle_for_end(&queue_item->call.for_e, &n, &for_loop_nesting, for_loop_indices);
                            break;
                        case QUEUE_ITEM_TYPE_FILTER:
                            hotcall_handle_filter(&queue_item->call.fi);
                            break;
                        case QUEUE_ITEM_TYPE_DO_WHILE:
                            hotcall_handle_do_while(&queue_item->call.dw);
                            break;
                        default:
                            printf("Error, the default statement should never happen... %d\n", queue_item->type);
                    }
                }
                sm_ctx->hcall.is_done = true;
                sm_ctx->hcall.queue_length = 0;
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

    printf("Exiting shared memory poller\n");

    sm_ctx->hcall.queue_length = 0;
    sgx_spin_unlock(&sm_ctx->hcall.spinlock);
    sm_ctx->hcall.is_done = true;

    return 0;
} /* ecall_start_poller */
