#include "hotcall_bundler_t.h"  /* print_string */
#include "hotcall-trusted.h"
#include <stdio.h>
#include <ctype.h>
#include "assert.h"
#include "boolean_expression_translator.h"
#include "math.h"

void
hotcall_register_config(struct hotcall_config *config) {
    hotcall_config = config;
}


static inline bool
hotcall_handle_if(struct hotcall_if *tif, uint8_t *exclude_list, int pos, int exclude_list_len) {
    struct postfix_item output[strlen(tif->config->predicate_fmt)];
    unsigned int output_length;
    to_postfix(tif->config->predicate_fmt, tif->params, output, &output_length);
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
    struct hotcall_function fc;
    struct hotcall_functionconfig config = {
        .function_id = dw->config->function_id,
        .n_params = dw->config->body_n_params
    };
    fc.config = &config;
    for(int i = 0; i < fc.config->n_params; ++i) fc.args[i] = parse_argument(&dw->body_params[i], 0);
    while(true) {
        if(!evaluate_postfix(output, output_length, hotcall_config, 0)) {
            return;
        }
        hotcall_config->execute_function(fc.config->function_id, fc.args, NULL);
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
    unsigned int n_params = re->config->n_params - 1;
    struct parameter *accumulator = &re->params[n_params];
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
    void *args[n_params];
    for(int i = 0; i < in_len; ++i) {
        for(int j = 0; j < n_params; ++j) {
            if(re->params[j].type == VARIABLE_TYPE && i > 0) continue;
            args[j] = parse_argument(&re->params[j], i);
        }
        hotcall_config->execute_function(re->config->function_id, args, &ret);
        combine_result(re->config->op, accumulator, &ret, i);
    }
}




static inline void
hotcall_handle_map(struct hotcall_map *ma) {
    const unsigned int n_params = ma->config->n_params - 1;
    const struct parameter *params_in = NULL, *params_out;
    for(int i = 0; i < n_params; ++i) {
        if(ma->params[i].type == VECTOR_TYPE) params_in = &ma->params[i];
    }
    sgx_assert(params_in != NULL, "Map input parameters contains no vector.");
    params_out = &ma->params[n_params];
    void *ret, *args[n_params];
    for(int i = 0; i < *params_in->value.vector.len; ++i) {
        for(int j = 0; j < ma->config->n_params; ++j) {
            if(ma->params[j].type == VARIABLE_TYPE && i > 0) continue;
            args[j] = parse_argument(&ma->params[j], i);
        }
        ret = parse_argument(params_out, i);
        hotcall_config->execute_function(ma->config->function_id, args, ret);
    }
}

static inline void
hotcall_handle_filter(struct hotcall_filter *fi) {
    struct vector_parameter *input_vec = NULL, *output_vec = NULL;
    struct parameter *input, *output;
    input = &fi->params[0];
    switch(input->type) {
        case FUNCTION_TYPE:
            for(int i = 0; i < input->value.function.n_params; ++i) {
                if(input->value.function.params[i].type != VECTOR_TYPE) continue;
                input_vec = &input->value.function.params[i].value.vector;
            }
            break;
        case VECTOR_TYPE:
            input_vec = &input->value.vector;
            break;
        default:
            break;
    }
    sgx_assert(input_vec != NULL, "ERROR, input parameter contains no vector. Undefined behaviour from now on..");

    output = &fi->params[fi->config->n_params - 1];
    switch(output->type) {
        case VECTOR_TYPE:
            output_vec = &output->value.vector;
            break;
        default:
            sgx_assert(true, "ERROR, return parameter is not of vector type. Undefined behaviour from now...");
    }
    sgx_assert(output_vec != NULL, "ERROR, return parameter is not of vector type. Undefined behaviour from now...");

    struct postfix_item postfix_output[strlen(fi->config->condition_fmt)];
    unsigned int postfix_output_length;
    to_postfix(fi->config->condition_fmt, fi->params, postfix_output, &postfix_output_length);
    int res, n_include = 0;
    for(int n = 0; n < *input_vec->len; ++n) {
        res = evaluate_postfix(postfix_output, postfix_output_length, hotcall_config, n);
        if(res) {
            switch(output_vec->fmt) {
                case 'u':
                    ((unsigned int *) output_vec->arg)[n_include] = ((unsigned int *) input_vec->arg)[n];
                    break;
                case 'b':
                    ((bool *) output_vec->arg)[n_include] = ((bool *) input_vec->arg)[n];
                    break;
                case 'd':
                    ((int *) output_vec->arg)[n_include] = ((int *) input_vec->arg)[n];
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
    unsigned int n_params = tor->config->n_params;

    struct parameter *params_in = NULL;
    for(int i = 0; i < n_params; ++i) {
        if(tor->params[i].type == VECTOR_TYPE) params_in = &tor->params[i];
    }
    sgx_assert(params_in != NULL, "For each input parameters contains no vector. Undefined behaviour!");

    void *args[n_params];
    for(int offset = 0; offset < *params_in->value.vector.len; ++offset) {
        for(int j = 0; j < n_params; ++j) {
            if(tor->params[j].type == VARIABLE_TYPE && offset > 0) continue;
            args[j] = parse_argument(&tor->params[j], offset);
        }
        hotcall_config->execute_function(tor->config->function_id, args, NULL);
    }
}

static inline void
update_loop_body_vector_variables(struct ecall_queue_item **queue, struct loop_stack_item *loop_stack, unsigned int *loop_stack_pos, int queue_pos) {
    struct ecall_queue_item *it;
    struct hotcall_function *fc;
    // Calculate parameter offset
    int offset = 0, power = 0;
    for(int k = *loop_stack_pos - 1; k >= 0; --k) {
        offset += loop_stack[k].index * pow(loop_stack[k].n_iters, power++);
    }
    unsigned int nesting = 0;
    for(it = queue[queue_pos + 1]; it->type != QUEUE_ITEM_TYPE_LOOP_END || nesting > 0; ++it) {
        if(it->type == QUEUE_ITEM_TYPE_FOR_BEGIN) nesting++;
        if(it->type == QUEUE_ITEM_TYPE_LOOP_END) nesting--;
        if(nesting > 0) {
            continue;
        }
        switch(it->type) {
            case QUEUE_ITEM_TYPE_FUNCTION:
                for(int j = 0; j < it->call.fc.config->n_params; ++j) {
                    fc = &it->call.fc;
                    if(fc->params[j].type != VECTOR_TYPE) continue;
                    fc->args[j] = parse_argument(&fc->params[j], offset);
                }
                break;
            case QUEUE_ITEM_TYPE_ASSIGN_VAR:
                it->call.var.offset = offset;
                break;
            case QUEUE_ITEM_TYPE_ASSIGN_PTR:
                it->call.ptr.offset = offset;
                break;
            default:
                continue;
        }
    }
}


static inline void
hotcall_handle_for_begin(struct hotcall_for_start *for_s, struct loop_stack_item *loop_stack, unsigned int *loop_stack_pos, uint8_t *exclude_list, int queue_pos, struct ecall_queue_item **queue) {

    if(!for_s->config->loop_in_process) {
        for_s->config->loop_in_process = true;
        loop_stack[*loop_stack_pos].body_len = for_s->config->body_len;
        loop_stack[*loop_stack_pos].index = 0;
        loop_stack[*loop_stack_pos].n_iters = *for_s->config->n_iters;
        (*loop_stack_pos)++;
    }

    bool continue_loop = loop_stack[*loop_stack_pos - 1].index < *for_s->config->n_iters;
    if(continue_loop) {
        if(for_s->config->loop_in_process) {
            memset(exclude_list + queue_pos, 0, loop_stack[*loop_stack_pos - 1].body_len + 2);
        }
        update_loop_body_vector_variables(queue, loop_stack, loop_stack_pos, queue_pos);
        return;
    }
    memset(exclude_list + queue_pos, 1, loop_stack[*loop_stack_pos - 1].body_len + 2);
    for_s->config->loop_in_process = false;
    (*loop_stack_pos)--;
}


static inline void
hotcall_handle_while_begin(struct hotcall_while_start *while_s, struct loop_stack_item *loop_stack, unsigned int *loop_stack_pos, uint8_t *exclude_list, int queue_pos, struct ecall_queue_item **queue) {

    if(!while_s->config->loop_in_process) {
        while_s->config->loop_in_process = true;
        loop_stack[*loop_stack_pos].body_len = while_s->config->body_len;
        loop_stack[*loop_stack_pos].index = 0;
        (*loop_stack_pos)++;
    }

    struct postfix_item output[strlen(while_s->config->predicate_fmt)];
    unsigned int output_length;
    to_postfix(while_s->config->predicate_fmt, while_s->params, output, &output_length);
    int res = evaluate_postfix(output, output_length, hotcall_config, 0);
    if(res) {
        memset(exclude_list + queue_pos, 0, loop_stack[*loop_stack_pos - 1].body_len + 2);
        return;
    }
    while_s->config->loop_in_process = false;
    memset(exclude_list + queue_pos, 1, loop_stack[*loop_stack_pos - 1].body_len + 2);
    --(*loop_stack_pos);
}

static inline void
hotcall_end_loop(struct loop_stack_item *loop_stack, unsigned int loop_stack_pos, int *pos) {
    *pos = *pos - (loop_stack[loop_stack_pos - 1].body_len + 2);
    loop_stack[loop_stack_pos - 1].index++;
}

static inline void
hotcall_handle_assign_var(struct hotcall_assign_variable *var) {
    switch(var->dst->value.variable.fmt) {
        case 'd': *(int *) var->dst->value.variable.arg = ((int *) var->src->value.vector.arg)[var->offset]; break;
        case 'u': *(unsigned int *) var->dst->value.variable.arg = ((unsigned int *) var->src->value.vector.arg)[var->offset]; break;
        case 'b': *(bool *) var->dst->value.variable.arg = ((bool *) var->src->value.vector.arg)[var->offset]; break;
        default: SWITCH_DEFAULT_REACHED
    }
}

static inline void
hotcall_handle_assign_ptr(struct hotcall_assign_pointer *ptr) {
    *(void **) ptr->dst->value.variable.arg = parse_argument(ptr->src, ptr->offset);
}

int
ecall_start_poller(struct shared_memory_ctx *sm_ctx){

    struct ecall_queue_item *queue_item;
    struct hotcall_function *fc;

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
                            fc = &queue_item->call.fc;
                            for(int i = 0; i < fc->config->n_params; ++i) {
                                switch(fc->params[i].type) {
                                    case VARIABLE_TYPE: fc->args[i] = fc->params[i].value.variable.arg; break;
                                    case POINTER_TYPE: fc->args[i] = *fc->params[i].value.pointer.arg; break;
                                    default: break;
                                }
                            }
                            hotcall_config->execute_function(fc->config->function_id, fc->args, fc->return_value);
                            break;
                        case QUEUE_ITEM_TYPE_IF: case QUEUE_ITEM_TYPE_IF_NULL:
                            hotcall_handle_if(&queue_item->call.tif, exclude_list, n, queue_length);
                            break;
                        case QUEUE_ITEM_TYPE_FOR_EACH:
                            hotcall_handle_for_each(&queue_item->call.tor);
                            break;
                        case QUEUE_ITEM_TYPE_FOR_BEGIN:
                            hotcall_handle_for_begin(&queue_item->call.for_s, loop_stack, &loop_stack_pos, exclude_list, n, sm_ctx->hcall.batch.queue);
                            break;
                        case QUEUE_ITEM_TYPE_FILTER:
                            hotcall_handle_filter(&queue_item->call.fi);
                            break;
                        case QUEUE_ITEM_TYPE_DO_WHILE:
                            hotcall_handle_do_while(&queue_item->call.dw);
                            break;
                        case QUEUE_ITEM_TYPE_WHILE_BEGIN:
                            hotcall_handle_while_begin(&queue_item->call.while_s, loop_stack, &loop_stack_pos, exclude_list, n, sm_ctx->hcall.batch.queue);
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
                        case QUEUE_ITEM_TYPE_ASSIGN_VAR:
                            hotcall_handle_assign_var(&queue_item->call.var);
                            break;
                        case QUEUE_ITEM_TYPE_ASSIGN_PTR:
                            hotcall_handle_assign_ptr(&queue_item->call.ptr);
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
