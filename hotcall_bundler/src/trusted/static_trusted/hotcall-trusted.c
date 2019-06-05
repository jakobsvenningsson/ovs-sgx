#include "hotcall_bundler_t.h"  /* print_string */
#include "hotcall-trusted.h"
#include <stdio.h>
#include <ctype.h>
#include "boolean_expression_translator.h"

void
hotcall_register_config(struct hotcall_config *config) {
    hotcall_config = config;
}


static inline bool
hotcall_handle_if(struct hotcall_if *tif, uint8_t *exclude_list, int pos, int exclude_list_len) {
    struct postfix_item output[strlen(tif->args->predicate.fmt)];
    unsigned int output_length;
    to_postfix(&tif->args->predicate, output, &output_length);
    int res = evaluate_postfix(output, output_length, hotcall_config);

    if(res == tif->args->predicate.expected && tif->args->else_branch_len > 0) {
        exclude_else_branch(exclude_list, pos, tif->args->then_branch_len, tif->args->else_branch_len);
    } else if(res != tif->args->predicate.expected) {
        if(tif->args->then_branch_len > 0) {
            exclude_if_branch(exclude_list, pos, tif->args->then_branch_len);
        }
        if(tif->args->return_if_false) {
            exclude_rest(exclude_list, pos, tif->args->then_branch_len ,tif->args->else_branch_len, exclude_list_len);
        }
    }
}

static inline void
hotcall_set_function_in_arguments(const struct function_parameters_in *params_in, struct hotcall_function *fc, int iter) {
    fc->args.n_args = params_in->n_params;
    struct function_parameter *param;
    int offset;
    for(int n = 0; n < params_in->n_params; ++n) {
        param = &params_in->params[n];
        offset = param->iter ? iter : 0;
        switch(param->fmt) {
            case 'u':
                fc->args.args[n] = ((unsigned int *) param->arg) + offset;
                break;
            case 'b':
                fc->args.args[n] = ((bool *) param->arg) + offset;
                break;
            case 'd':
                fc->args.args[n] = ((int *) param->arg) + offset;
                break;
            default:
                printf("Switch default at %s %d.\n", __FILE__, __LINE__);
                break;
        }
    }
}

static inline void
hotcall_set_function_out_arguments(struct function_filter_out *params_out, const struct function_parameters_in *params_in, struct hotcall_function *fc, int iter, int n_include) {
    struct function_parameter *param_i, *param_o;
    int offset;
    for(int n = 0; n < params_in->n_params; ++n) {
        param_i = &params_in->params[n];
        param_o = &params_out->params[n];
        offset = param_i->iter ? iter : 0;
        switch(param_i->fmt) {
            case 'u':
                ((unsigned int *) param_o->arg)[n_include] = ((unsigned int *) param_i->arg)[offset];
                break;
            case 'b':
                ((bool *) param_o->arg)[n_include] = ((bool *) param_i->arg)[offset];
                break;
            case 'd':
                ((int *) param_o->arg)[n_include] = ((int *) param_i->arg)[offset];
                break;
            default:
                printf("Switch default at %s %d.\n", __FILE__, __LINE__);
                break;
        }
    }
}

static inline void
hotcall_handle_do_while(struct hotcall_do_while *dw) {
    struct postfix_item output[strlen(dw->args->predicate.fmt)];
    unsigned int output_length;
    to_postfix(&dw->args->predicate, output, &output_length);

    struct hotcall_function *fc, fc_body;
    fc_body.id = dw->f;
    hotcall_set_function_in_arguments(&dw->args->params, &fc_body, 0);
    unsigned int res;
    while(true) {
        res = evaluate_postfix(output, output_length, hotcall_config);
        if(res != dw->args->predicate.expected) {
            return;
        }
        hotcall_config->execute_function(&fc_body);
    }
}


static inline void
hotcall_handle_map(struct hotcall_map *ma) {
    const struct function_parameters_in *params_in;
    struct function_map_out *params_out;
    params_in = &ma->args->params_in;
    params_out = &ma->args->params_out;

    struct hotcall_function fc;
    fc.id = ma->f;
    fc.args.n_args = params_in->n_params;

    for(int i = 0; i < params_in->iters; ++i) {
        hotcall_set_function_in_arguments(params_in, &fc, i);
        switch(params_out->fmt) {
            case 'u':
                fc.return_value = ((unsigned int *) params_out->params) + i;
                break;
            case 'b':
                fc.return_value = ((bool *) params_out->params) + i;
                break;
            case 'd':
                fc.return_value = ((int *) params_out->params) + i;
                break;
            default:
                printf("Switch default at %s %d.\n", __FILE__, __LINE__);
                break;
        }
        hotcall_config->execute_function(&fc);
    }
}

static inline void
hotcall_handle_filter(struct hotcall_filter *fi) {
    struct postfix_item output[strlen(fi->args->predicate.fmt)];
    unsigned int output_length;
    to_postfix(&fi->args->predicate, output, &output_length);

    struct hotcall_function *fc;
    unsigned int res, n_include = 0;
    for(int i = 0; i < fi->args->params_in.n_params; ++i) {
        for(int j = 0; j < fi->args->predicate.n_variables; ++j) {
            if(fi->args->predicate.variables[j].type == FUNCTION_TYPE) {
                fc = (struct hotcall_function *) fi->args->predicate.variables[j].val;
                fc->args.n_args = fi->args->params_in.n_params;
                hotcall_set_function_in_arguments(&fi->args->params_in, fc, i);
            }
        }
        res = evaluate_postfix(output, output_length, hotcall_config);
        if(res == fi->args->predicate.expected) {
            hotcall_set_function_out_arguments(&fi->args->params_out, &fi->args->params_in, fc, i, n_include++);
        }
        *(fi->args->params_out.len) = n_include;
    }
}

static inline void
hotcall_handle_for_each(struct hotcall_for_each *tor) {
    struct hotcall_function fc;
    fc.id = tor->f;
    fc.args.n_args = tor->args->params_in.n_params;
    struct function_parameter *param;
    int offset;
    for(int i = 0; i < tor->args->params_in.iters; ++i) {
        for(int j = 0; j < tor->args->params_in.n_params; ++j) {
            param = &tor->args->params_in.params[j];
            offset = param->iter ? i : 0;
            switch(param->fmt) {
                case 'd':
                    fc.args.args[j] = (int *) param->arg + i;
                    break;
                case 'u':
                    fc.args.args[j] = (unsigned int *) param->arg + i;
                    break;
                case 'b':
                    fc.args.args[j] = (bool *) param->arg + i;
                        break;
                default:
                    break;
            }
        }
        hotcall_config->execute_function(&fc);
    }
}

static inline void
hotcall_handle_for_begin(struct hotcall_for_start *for_s, unsigned int *for_loop_nesting, uint8_t *exclude_list, int pos) {
    bool continue_loop = for_s->args->n_iters-- > 0;
    if(continue_loop) {
        (*for_loop_nesting)++;
    } else {
        for_loop_indices[*for_loop_nesting] = 0;
    }
    memset(exclude_list + pos + 1, continue_loop ? 0 : 1, for_s->args->n_rows + 1);
}

static inline void
hotcall_handle_for_end(struct hotcall_for_end *for_e, int *pos, unsigned int *for_loop_nesting, unsigned int *for_loop_indices) {
    *pos = *pos - (for_e->args->n_rows + 2);
    (*for_loop_nesting)--;
    for_loop_indices[*for_loop_nesting]++;
}

static inline void
hotcall_handle_while_begin(struct hotcall_while_start *while_s, unsigned int *for_loop_nesting, uint8_t *exclude_list, int pos) {
    struct postfix_item output[strlen(while_s->args->predicate.fmt)];
    unsigned int output_length;
    to_postfix(&while_s->args->predicate, output, &output_length);
    int res = evaluate_postfix(output, output_length, hotcall_config);
    if(res == while_s->args->predicate.expected) {
        return;
    }
    memset(exclude_list + pos + 1, 1, while_s->args->n_rows + 1);
}

static inline void
hotcall_handle_while_end(struct hotcall_while_end *while_e, int *pos, unsigned int *for_loop_nesting, unsigned int *for_loop_indices) {
    *pos = *pos - (while_e->args->n_rows + 2);
    //(*for_loop_nesting)--;
    //for_loop_indices[*for_loop_nesting]++;
}


int
ecall_start_poller(struct shared_memory_ctx *sm_ctx){

    struct ecall_queue_item *queue_item, *queue_item_next;
    struct hotcall_function *fc, *prev_fc;

    while (1) {

        sgx_spin_lock(&sm_ctx->hcall.spinlock);

        if (sm_ctx->hcall.run) {

            sm_ctx->hcall.run = false;

            uint8_t exclude_list[sm_ctx->hcall.queue_length];
            memset(exclude_list, 0, sm_ctx->hcall.queue_length);


            int n;
            for(n = 0; n < sm_ctx->hcall.queue_length; ++n) {
                    queue_item = sm_ctx->hcall.ecall_queue[n];
                    if(exclude_list[n]) {
                        //printf("excluding %d\n", queue_item->type);
                        continue;
                    }

                    switch(queue_item->type) {
                        case QUEUE_ITEM_TYPE_DESTROY:
                            goto exit;
                        case QUEUE_ITEM_TYPE_FUNCTION:
                            hotcall_handle_function(&queue_item->call.fc);
                            prev_fc = &queue_item->call.fc;
                            break;
                        case QUEUE_ITEM_TYPE_IF: case QUEUE_ITEM_TYPE_IF_NULL:
                            hotcall_handle_if(&queue_item->call.tif, exclude_list, n, sm_ctx->hcall.queue_length);
                            break;
                        case QUEUE_ITEM_TYPE_FOR_EACH:
                            hotcall_handle_for_each(&queue_item->call.tor);
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
                        case QUEUE_ITEM_TYPE_WHILE_BEGIN:
                            hotcall_handle_while_begin(&queue_item->call.while_s, &for_loop_nesting, exclude_list, n);
                            break;
                        case QUEUE_ITEM_TYPE_WHILE_END:
                            hotcall_handle_while_end(&queue_item->call.while_e, &n, &for_loop_nesting, for_loop_indices);
                            break;
                        case QUEUE_ITEM_TYPE_MAP:
                            hotcall_handle_map(&queue_item->call.ma);
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

    sm_ctx->hcall.queue_length = 0;
    sgx_spin_unlock(&sm_ctx->hcall.spinlock);
    sm_ctx->hcall.is_done = true;

    return 0;
} /* ecall_start_poller */
