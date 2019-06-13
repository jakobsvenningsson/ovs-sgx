#include "hotcall_bundler_t.h"  /* print_string */
#include "hotcall-trusted.h"
#include <stdio.h>
#include <ctype.h>
#include "assert.h"
#include "boolean_expression_translator.h"

void
hotcall_register_config(struct hotcall_config *config) {
    hotcall_config = config;
}

static inline bool
hotcall_handle_if(struct hotcall_if *tif, uint8_t *exclude_list, int pos, int exclude_list_len) {
    struct postfix_item output[strlen(tif->config->predicate_fmt)];
    unsigned int output_length;
    to_postfix(tif->config->predicate_fmt, tif->args, output, &output_length);
    int res = evaluate_postfix(output, output_length, hotcall_config, 0);
    if(res && tif->config->else_branch_len > 0) {
        exclude_else_branch(exclude_list, pos, tif->config->then_branch_len, tif->config->else_branch_len);
    } else if(!res) {
        if(tif->config->then_branch_len > 0) {
            exclude_if_branch(exclude_list, pos, tif->config->then_branch_len);
        }
        if(tif->config->return_if_false) {
            exclude_rest(exclude_list, pos, tif->config->then_branch_len ,tif->config->else_branch_len, exclude_list_len);
        }
    }
}

static inline void
hotcall_handle_do_while(struct hotcall_do_while *dw) {
    struct postfix_item output[strlen(dw->config->condition_fmt)];
    unsigned int output_length;
    to_postfix(dw->config->condition_fmt, dw->condition_params, output, &output_length);
    struct hotcall_function_ fc;
    struct hotcall_function_config config = {
        .function_id = dw->config->function_id,
        .n_params = dw->config->body_n_params
    };
    fc.config = &config;
    fc.params = dw->body_params;;
    while(true) {
        if(!evaluate_postfix(output, output_length, hotcall_config, 0)) {
            return;
        }
        hotcall_config->execute_function(&fc);
    }
}

static inline void
combine_result(char op, struct parameter *accumulator, void *ret, int n) {

    if(n == 0) {
        switch(accumulator->value.variable.fmt) {
            case 'd': case 'b':
                *(int *) accumulator->value.variable.arg = *(int *) ret;
                break;
            case 'u':
                *(unsigned int *) accumulator->value.variable.arg = *(unsigned int *) ret;
                break;
            default:
                SWITCH_DEFAULT_REACHED
        }
        return;
    }

    switch(op) {
        case '+':
            switch(accumulator->value.variable.fmt) {
                case 'd': case 'b':
                    *(int *) accumulator->value.variable.arg += *(int *) ret;
                    break;
                case 'u':
                    *(unsigned int *) accumulator->value.variable.arg += *(unsigned int *) ret;
                    break;
                default:
                    SWITCH_DEFAULT_REACHED
            }
            break;
        case '-':
            switch(accumulator->value.variable.fmt) {
                case 'd': case 'b':
                    *(int *) accumulator->value.variable.arg -= *(int *) ret;
                    break;
                case 'u':
                    *(unsigned int *) accumulator->value.variable.arg -= *(unsigned int *) ret;
                    break;
                default:
                    SWITCH_DEFAULT_REACHED
            }
            break;
        case '&':
            switch(accumulator->value.variable.fmt) {
                case 'd': case 'b':
                    *(int *) accumulator->value.variable.arg = *(int *) accumulator->value.variable.arg && *(int *) ret;
                    break;
                case 'u':
                    *(unsigned int *) accumulator->value.variable.arg = *(unsigned int *) accumulator->value.variable.arg  && *(unsigned int *) ret;
                    break;
                default:
                    SWITCH_DEFAULT_REACHED
            }
            break;
        case '|':
            switch(accumulator->value.variable.fmt) {
                case 'd': case 'b':
                    *(int *) accumulator->value.variable.arg = *(int *) accumulator->value.variable.arg || *(int *) ret;
                    break;
                case 'u':
                    *(unsigned int *) accumulator->value.variable.arg =  *(unsigned int *) accumulator->value.variable.arg || *(unsigned int *) ret;
                    break;
                default:
                    SWITCH_DEFAULT_REACHED
            }
            break;
        default:
            SWITCH_DEFAULT_REACHED
    }
}

static inline void
hotcall_handle_reduce(struct hotcall_reduce *re) {
    printf("hotcall_handle_reduce\n");
    struct parameter *accumulator = &re->params[re->config->n_params - 1];
    unsigned int in_len = *re->params[0].value.vector.len;

    // No function is used in reduce, we shall only combine the input elements.
    if(re->config->function_id == 255) {
        for(int i = 0; i < in_len; ++i) {
            switch(re->params[0].value.variable.fmt) {
                case 'd':
                    combine_result(re->config->op, accumulator, ((int *) re->params[0].value.variable.arg) + i, i);

                    break;
                case 'u':
                    combine_result(re->config->op, accumulator, ((unsigned int *) re->params[0].value.variable.arg) + i, i);

                    break;
                case 'b':
                    combine_result(re->config->op, accumulator, ((bool *) re->params[0].value.variable.arg) + i, i);
                    break;
                default: SWITCH_DEFAULT_REACHED
            }
        }
        return;
    }

    int ret = 0;
    for(int i = 0; i < in_len; ++i) {
        struct hotcall_function_ fc;
        struct hotcall_function_config config = {
            .function_id = re->config->function_id,
            .n_params = re->config->n_params - 1,
        };
        fc.config = &config;
        fc.params = re->params;
        fc.return_value = &ret;
        struct vector_parameter *vec_param;
        for(int j = 0; j < fc.config->n_params && i > 0; ++j) {
            if(re->params[j].type != VECTOR_TYPE_) continue;
            vec_param = &re->params[j].value.vector;
            switch(vec_param->fmt) {
                case 'd':
                    vec_param->arg = ((int *) vec_param->arg) + 1;
                    break;
                case 'u':
                    vec_param->arg = ((unsigned int *) vec_param->arg) + 1;
                    break;
                case 'b':
                    vec_param->arg = ((bool *) vec_param->arg) + 1;
                    break;
                default: SWITCH_DEFAULT_REACHED
            }
        }
        hotcall_config->execute_function(&fc);
        combine_result(re->config->op, accumulator, &ret, i);
    }
}


static inline void
hotcall_handle_map(struct hotcall_map *ma) {
    const struct vector_parameter *params_in_vec, *params_out_vec;
    const struct parameter *params_in;
    params_in = &ma->params[0];
    params_in_vec = &params_in->value.vector;
    params_out_vec = &ma->params[ma->config->n_params - 1].value.vector;

    struct hotcall_function_ fc;
    struct hotcall_function_config config = {
        .function_id = ma->config->function_id,
        .n_params = ma->config->n_params - 1,
    };
    fc.config = &config;
    fc.params = ma->params;

    for(int i = 0; i < *params_in_vec->len; ++i) {
        struct vector_parameter *vec_param;
        for(int j = 0; j < fc.config->n_params && i > 0; ++j) {
            if(ma->params[j].type != VECTOR_TYPE_) continue;
            vec_param = &ma->params[j].value.vector;
            switch(vec_param->fmt) {
                case 'd':
                    vec_param->arg = ((int *) vec_param->arg) + 1;
                    break;
                case 'u':
                    vec_param->arg = ((unsigned int *) vec_param->arg) + 1;
                    break;
                case 'b':
                    vec_param->arg = ((bool *) vec_param->arg) + 1;
                    break;
                default: SWITCH_DEFAULT_REACHED
            }
        }

        switch(params_out_vec->fmt) {
            case 'u':
                fc.return_value = ((unsigned int *) params_out_vec->arg) + i;
                break;
            case 'b':
                fc.return_value = ((bool *) params_out_vec->arg) + i;
                break;
            case 'd':
                fc.return_value = ((int *) params_out_vec->arg) + i;
                break;
            default:
                SWITCH_DEFAULT_REACHED
                break;
        }
        hotcall_config->execute_function(&fc);
    }
}

static inline void
hotcall_handle_filter(struct hotcall_filter *fi) {
    struct vector_parameter *input_vec = NULL, *output_vec = NULL;
    struct parameter *input, *output;
    input = &fi->params[0];
    switch(input->type) {
        case FUNCTION_TYPE_:
            for(int i = 0; i < input->value.function.n_params; ++i) {
                if(input->value.function.params[i].type != VECTOR_TYPE_) continue;
                input_vec = &input->value.function.params[i].value.vector;
            }
            break;
        case VECTOR_TYPE_:
            input_vec = &input->value.vector;
            break;
        default:
            break;
    }
    sgx_assert(input_vec != NULL, "ERROR, input parameter contains no iterator. Undefined behaviour from now on..");

    output = &fi->params[fi->config->n_params - 1];
    switch(output->type) {
        case VECTOR_TYPE_:
            output_vec = &output->value.vector;
            break;
        default:
            sgx_assert(true, "ERROR, return parameter is not of variable type. Undefined behaviour from now...");
    }
    sgx_assert(output_vec != NULL, "ERROR, return parameter is not of variable type. Undefined behaviour from now...");

    struct postfix_item postfix_output[strlen(fi->config->condition_fmt)];
    unsigned int postfix_output_length;
    to_postfix(fi->config->condition_fmt, fi->params, postfix_output, &postfix_output_length);
    int res, offset, n_include = 0;
    for(int n = 0; n < *input_vec->len; ++n) {
        res = evaluate_postfix(postfix_output, postfix_output_length, hotcall_config, n);
        if(res) {
            offset = n;
            switch(output_vec->fmt) {
                case 'u':
                    ((unsigned int *) output_vec->arg)[n_include] = ((unsigned int *) input_vec->arg)[offset];
                    break;
                case 'b':
                    ((bool *) output_vec->arg)[n_include] = ((bool *) input_vec->arg)[offset];
                    break;
                case 'd':
                    ((int *) output_vec->arg)[n_include] = ((int *) input_vec->arg)[offset];
                    break;
                default:
                    SWITCH_DEFAULT_REACHED
                    break;
            }
            n_include++;
        }
    }
    *(output_vec->len) = n_include;

}

static inline void
hotcall_handle_for_each(struct hotcall_for_each *tor) {
    struct hotcall_function_ fc;
    struct hotcall_function_config config = {
        .function_id = tor->config->function_id,
        .n_params = tor->config->n_params
    };
    fc.config = &config;
    fc.params = tor->params;

    for(int i = 0; i < *tor->params[0].value.vector.len; ++i) {
        struct vector_parameter *vec_param;
        for(int j = 0; j < fc.config->n_params && i > 0; ++j) {
            if(tor->params[j].type != VECTOR_TYPE_) continue;
            vec_param = &tor->params[j].value.vector;
            switch(vec_param->fmt) {
                case 'd':
                    vec_param->arg = ((int *) vec_param->arg) + 1;
                    break;
                case 'u':
                    vec_param->arg = ((unsigned int *) vec_param->arg) + 1;
                    break;
                case 'b':
                    vec_param->arg = ((bool *) vec_param->arg) + 1;
                    break;
                default: SWITCH_DEFAULT_REACHED
            }
        }
        hotcall_config->execute_function(&fc);
    }
}

static inline void
hotcall_handle_for_begin(struct hotcall_for_start *for_s, struct loop_stack_item *loop_stack, unsigned int *loop_stack_pos, uint8_t *exclude_list, int pos, int batch_len, struct ecall_queue_item **queue) {
    bool continue_loop = *loop_stack_pos == 0 ? true : loop_stack[*loop_stack_pos - 1].index < *for_s->config->n_iters - 1;
    if(continue_loop) {
        if(for_s->config->loop_in_process == false) {
            for_s->config->loop_in_process = true;
            loop_stack[*loop_stack_pos].body_len = for_s->config->body_len;
            loop_stack[*loop_stack_pos].index = 0;
            (*loop_stack_pos)++;
        } else {
            memset(exclude_list + pos, 0, loop_stack[*loop_stack_pos - 1].body_len + 2);
            loop_stack[*loop_stack_pos - 1].index++;

            struct ecall_queue_item *it;
            struct parameter *param;
            for(it = queue[pos + 1]; it != *queue + batch_len; ++it) {
                if(it->type != QUEUE_ITEM_TYPE_FUNCTION) {
                    continue;
                }
                for(int j = 0; j < it->call.fc_.config->n_params; ++j) {
                    param = &it->call.fc_.params[j];
                    if(param->type != VECTOR_TYPE_) {
                        continue;
                    }
                    switch(param->value.vector.fmt) {
                        case 'd':
                            param->value.vector.arg = ((int *) param->value.vector.arg) + 1;
                            break;
                        case 'u':
                            param->value.vector.arg = ((unsigned int *) param->value.vector.arg) + 1;
                            break;
                        case 'b':
                            param->value.vector.arg = ((bool *) param->value.vector.arg) + 1;
                            break;
                        default: SWITCH_DEFAULT_REACHED
                    }
                }
            }
        }
        return;
    }
    memset(exclude_list + pos, 1, loop_stack[*loop_stack_pos - 1].body_len + 2);
    for_s->config->loop_in_process = false;
    (*loop_stack_pos)--;
}


static inline void
hotcall_handle_while_begin(struct hotcall_while_start *while_s, struct loop_stack_item *loop_stack, unsigned int *loop_stack_pos, uint8_t *exclude_list, int pos) {
    struct postfix_item output[strlen(while_s->config->predicate_fmt)];
    unsigned int output_length;
    to_postfix(while_s->config->predicate_fmt, while_s->params, output, &output_length);
    int res = evaluate_postfix(output, output_length, hotcall_config, 0);
    if(res) {
        if(while_s->config->loop_in_process == false) {
            while_s->config->loop_in_process = true;
            loop_stack[*loop_stack_pos].body_len = while_s->config->body_len;
            loop_stack[*loop_stack_pos].index = 0;
            (*loop_stack_pos)++;
        } else {
            memset(exclude_list + pos, 0, loop_stack[*loop_stack_pos - 1].body_len + 2);
            loop_stack[*loop_stack_pos - 1].index++;
        }
        return;
    }
    while_s->config->loop_in_process = false;
    memset(exclude_list + pos, 1, loop_stack[*loop_stack_pos - 1].body_len + 2);
    --(*loop_stack_pos);
}

static inline void
hotcall_end_loop(struct loop_stack_item *loop_stack, unsigned int loop_stack_pos, int *pos) {
    *pos = *pos - (loop_stack[loop_stack_pos - 1].body_len + 2);
}

int
ecall_start_poller(struct shared_memory_ctx *sm_ctx){

    struct ecall_queue_item *queue_item;
    struct hotcall_function *fc;
    struct hotcall_function_ *fc_;


    while (1) {

        sgx_spin_lock(&sm_ctx->hcall.spinlock);

        if (sm_ctx->hcall.run) {

            sm_ctx->hcall.run = false;

            uint8_t exclude_list[sm_ctx->hcall.batch.queue_len];
            memset(exclude_list, 0, sm_ctx->hcall.batch.queue_len);

            struct loop_stack_item loop_stack[MAX_LOOP_RECURSION] = { 0 };
            unsigned int loop_stack_pos = 0;

            int n;

            unsigned int queue_length = sm_ctx->hcall.batch.queue_len;

            for(n = 0; n < queue_length; ++n) {
                    queue_item = sm_ctx->hcall.batch.queue[n];
                    if(exclude_list[n]) {
                        continue;
                    }

                    switch(queue_item->type) {
                        case QUEUE_ITEM_TYPE_DESTROY:
                            goto exit;
                        case QUEUE_ITEM_TYPE_FUNCTION:
                            hotcall_config->execute_function(&queue_item->call.fc_);
                            break;
                        case QUEUE_ITEM_TYPE_IF: case QUEUE_ITEM_TYPE_IF_NULL:
                            hotcall_handle_if(&queue_item->call.tif, exclude_list, n, queue_length);
                            break;
                        case QUEUE_ITEM_TYPE_FOR_EACH:
                            hotcall_handle_for_each(&queue_item->call.tor);
                            break;
                        case QUEUE_ITEM_TYPE_FOR_BEGIN:
                            hotcall_handle_for_begin(&queue_item->call.for_s, loop_stack, &loop_stack_pos, exclude_list, n, queue_length, sm_ctx->hcall.batch.queue);
                            break;
                        case QUEUE_ITEM_TYPE_FILTER:
                            hotcall_handle_filter(&queue_item->call.fi);
                            break;
                        case QUEUE_ITEM_TYPE_DO_WHILE:
                            hotcall_handle_do_while(&queue_item->call.dw);
                            break;
                        case QUEUE_ITEM_TYPE_WHILE_BEGIN:
                            hotcall_handle_while_begin(&queue_item->call.while_s, loop_stack, &loop_stack_pos, exclude_list, n);
                            break;
                        case QUEUE_ITEM_TYPE_LOOP_END:
                            hotcall_end_loop(loop_stack, loop_stack_pos, &n);
                            break;
                        case QUEUE_ITEM_TYPE_MAP:
                            hotcall_handle_map(&queue_item->call.ma);
                            break;
                        case QUEUE_ITEM_TYPE_REDUCE:
                            hotcall_handle_reduce(&queue_item->call.re);
                            break;
                        case QUEUE_ITEM_TYPE_ERROR:
                            sm_ctx->hcall.batch.error = queue_item->call.err.error_code;
                            goto batch_done;
                        case QUEUE_ITEM_TYPE_IF_ELSE:
                            break;
                        default:
                            SWITCH_DEFAULT_REACHED
                    }
                }
                batch_done:
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
