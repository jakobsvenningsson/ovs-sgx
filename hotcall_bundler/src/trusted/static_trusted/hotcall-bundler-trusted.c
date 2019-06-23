#include "hotcall_bundler_t.h"  /* print_string */
#include "hotcall-bundler-trusted.h"
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
hotcall_handle_if(struct hotcall_if *tif, uint8_t *exclude_list, int pos, int exclude_list_len, int offset) {

    //struct postfix_item output[strlen(tif->config->predicate_fmt)];
    //unsigned int n;
    //n = to_postfix(tif->config->predicate_fmt, tif->params, output);

    int res = evaluate_postfix(tif->config->postfix, tif->config->postfix_length, hotcall_config, offset); //

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
    struct hotcall_function fc;
    struct hotcall_functionconfig config = {
        .function_id = dw->config->function_id,
        .n_params = dw->config->body_n_params
    };
    fc.config = &config;
    parse_function_arguments(dw->body_params, config.n_params, 0, fc.args);
    while(true) {
        if(!evaluate_postfix(dw->config->postfix, dw->config->postfix_length, hotcall_config, 0)) {
            return;
        }
        hotcall_config->execute_function(dw->config->function_id, fc.args, NULL);
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
            case ui8:
                *(uint8_t *) accumulator->value.variable.arg = *(uint8_t *) ret;
                break;
            case ui16:
                *(uint16_t *) accumulator->value.variable.arg = *(uint16_t *) ret;
                break;
            case ui32:
                *(uint32_t *) accumulator->value.variable.arg = *(uint32_t *) ret;
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
                case ui8:
                    *(uint8_t *) accumulator->value.variable.arg += *(uint8_t *) ret;
                    break;
                case ui16:
                    *(uint16_t *) accumulator->value.variable.arg += *(uint16_t *) ret;
                    break;
                case ui32:
                    *(uint32_t *) accumulator->value.variable.arg += *(uint32_t *) ret;
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
                case ui8:
                    *(uint8_t *) accumulator->value.variable.arg -= *(uint8_t *) ret;
                    break;
                case ui16:
                    *(uint16_t *) accumulator->value.variable.arg -= *(uint16_t *) ret;
                    break;
                case ui32:
                    *(uint32_t *) accumulator->value.variable.arg -= *(uint32_t *) ret;
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
                case ui8:
                    *(uint8_t *) accumulator->value.variable.arg && *(uint8_t *) ret;
                    break;
                case ui16:
                    *(uint16_t *) accumulator->value.variable.arg && *(uint16_t *) ret;
                    break;
                case ui32:
                    *(uint32_t *) accumulator->value.variable.arg && *(uint32_t *) ret;
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
                case ui8:
                    *(uint8_t *) accumulator->value.variable.arg || *(uint8_t *) ret;
                    break;
                case ui16:
                    *(uint16_t *) accumulator->value.variable.arg || *(uint16_t *) ret;
                    break;
                case ui32:
                    *(uint32_t *) accumulator->value.variable.arg || *(uint32_t *) ret;
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
            switch(re->params[0].value.vector.fmt) {
                case 'd':
                    combine_result(re->config->op, accumulator, ((int *) re->params[0].value.vector.arg) + i, i);
                    break;
                case 'u':
                    combine_result(re->config->op, accumulator, ((unsigned int *) re->params[0].value.vector.arg) + i, i);
                    break;
                case 'b':
                    combine_result(re->config->op, accumulator, ((bool *) re->params[0].value.vector.arg) + i, i);
                    break;
                case ui8:
                    combine_result(re->config->op, accumulator, ((uint8_t *) re->params[0].value.vector.arg) + i, i);
                    break;
                case ui16:
                    combine_result(re->config->op, accumulator, ((uint16_t *) re->params[0].value.vector.arg) + i, i);
                    break;
                case ui32:
                    combine_result(re->config->op, accumulator, ((uint32_t *) re->params[0].value.vector.arg) + i, i);
                    break;
                default: SWITCH_DEFAULT_REACHED
            }
        }
        return;
    }

    int ret = 0;
    void *args[in_len][n_params];
    parse_arguments(re->params, n_params, in_len, args);
    for(int i = 0; i < in_len; ++i) {
        hotcall_config->execute_function(re->config->function_id, args[i], &ret);
        combine_result(re->config->op, accumulator, &ret, i);
    }
}




static inline void
hotcall_handle_map(struct hotcall_map *ma) {
    const unsigned int n_params = ma->config->n_params, n_iters = *ma->config->n_iters;

    #ifdef SGX_DEBUG
    const struct parameter *params_in = NULL;
    for(int i = 0; i < n_params - 1; ++i) {
        if(ma->params[i].type == VECTOR_TYPE || ma->params[i].type == VECTOR_TYPE_v2) {
            params_in = &ma->params[i];
            break;
        }
    }
    sgx_assert(params_in != NULL, "Map input parameters contains no vector.");
    #endif


    void *args[n_iters][n_params];
    parse_arguments(ma->params, n_params, n_iters, args);
    for(int i = 0; i < n_iters; ++i) {
        hotcall_config->execute_function(ma->config->function_id, args[i], args[i][n_params - 1]);
    }
}

static inline void
hotcall_handle_filter(struct hotcall_filter *fi) {
    const unsigned int n_params = fi->config->n_params - 1;
    struct vector_parameter *input_vec = NULL, *output_vec = NULL;
    struct parameter *input, *output;
    for(int i = 0; i < n_params; ++i) {
        if(fi->params[i].type == VECTOR_TYPE || fi->params[i].type == FUNCTION_TYPE) {
            input = &fi->params[i];
            break;
        }
    }
    switch(input->type) {
        case FUNCTION_TYPE:
            for(int i = 0; i < input->value.function.n_params; ++i) {
                if(input->value.function.params[i].type != VECTOR_TYPE) continue;
                input_vec = &input->value.function.params[i].value.vector;
                input = &input->value.function.params[i];
                break;
            }
            break;
        case VECTOR_TYPE:
            input_vec = &input->value.vector;
            break;
        default:
            break;
    }
    sgx_assert(input_vec != NULL, "ERROR, input parameter contains no vector. Undefined behaviour from now on..");

    output = &fi->params[n_params];
    switch(output->type) {
        case VECTOR_TYPE:
            output_vec = &output->value.vector;
            break;
        default:
            sgx_assert(true, "ERROR, return parameter is not of vector type. Undefined behaviour from now...");
    }
    sgx_assert(output_vec != NULL, "ERROR, return parameter is not of vector type. Undefined behaviour from now...");

    int res, n_include = 0;
    for(int n = 0; n < *input_vec->len; ++n) {
        res = evaluate_postfix(fi->config->postfix, fi->config->postfix_length, hotcall_config, n);

        if(res) {
            switch(output_vec->fmt) {
                case 'u':
                    ((unsigned int *) output_vec->arg)[n_include] = *(unsigned int *) parse_argument_v2(input, n);
                    break;
                case 'b':
                    ((bool *) output_vec->arg)[n_include] = *(bool *) parse_argument_v2(input, n);
                    break;
                case 'd':
                    ((int *) output_vec->arg)[n_include] = *(int *) parse_argument_v2(input, n);
                    break;
                case 'p':
                    ((void **) output_vec->arg)[n_include] = *(void **) parse_argument_v2(input, n);
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
    unsigned int n_iters = *tor->config->n_iters;

    #ifdef SGX_DEBUG
    struct parameter *params_in = NULL;
    for(int i = 0; i < n_params; ++i) {
        if(tor->params[i].type == VECTOR_TYPE || tor->params[i].type == VECTOR_TYPE_v2) {
            params_in = &tor->params[i];
            break;
        }
    }
    sgx_assert(params_in != NULL, "For each input parameters contains no vector. Undefined behaviour!");
    #endif
    /*void *args[n_params]; int addr_modifications[10];
    for(int offset = 0; offset < *params_in->value.vector.len; ++offset) {
        for(int j = 0; j < n_params; ++j) {
            args[j] = parse_argument_1(&tor->params[j], addr_modifications, 0, offset);*/
            //if(tor->params[j].type == VARIABLE_TYPE && offset > 0) continue;
            /*switch(tor->params[j].type) {
                case VARIABLE_TYPE:
                    if(offset > 0) continue;
                    //args[j] = parse_argument_1(&tor->params[j], 0);//((char *) parse_argument_1(&tor->params[j], offset, tor->params[j].value.variable.access_member)); //+ tor->params[j].value.variable.access_member;
                    break;
                case VECTOR_TYPE:
                    //args[j] = parse_argument_1(&tor->params[j], 0);//((char *) parse_argument_1(&tor->params[j], offset, tor->params[j].value.vector.access_member)); // + tor->params[j].value.vector.access_member;
                    break;
                case VECTOR_TYPE_v2:
                    //args[j] = parse_argument_1(&tor->params[j], 0);//((char *) parse_argument_1(&tor->params[j], offset, tor->params[j].value.vector.access_member)); // + tor->params[j].value.vector.access_member;
                    break;
                default: SWITCH_DEFAULT_REACHED
            }*/
            //args[j] = parse_argument(&tor->params[j], offset);// + tor->params[j].value.vector.access_member;
            //printf("%p\n", args[j]);
    //    }
    //    hotcall_config->execute_function(tor->config->function_id, args, NULL);
    //}

    void *args[n_iters][n_params];
    parse_arguments(tor->params, n_params, n_iters, args);
    for(int i = 0; i < n_iters; ++i) {
        hotcall_config->execute_function(tor->config->function_id, args[i], args[i][n_params - 1]);
    }
}

static inline unsigned int
update_loop_body_vector_variables(struct loop_stack_item *loop_stack, unsigned int loop_stack_pos) {
    unsigned int offset = 0, power = 0;
    for(int k = loop_stack_pos; k >= 0; --k) {
        offset += loop_stack[k].index * pow(loop_stack[k].n_iters, power++);
    }
    return offset;
}


static inline void
hotcall_handle_for_begin(struct hotcall_for_start *for_s, struct loop_stack_item *loop_stack, unsigned int *loop_stack_pos, uint8_t *exclude_list, int queue_pos) {
    if(!for_s->config->loop_in_process) {
        for_s->config->loop_in_process = true;
        loop_stack[(*loop_stack_pos)].n_iters = *for_s->config->n_iters;
        /*loop_stack[(*loop_stack_pos)] = (struct loop_stack_item) {
                for_s->config->body_len,
                0,
                *for_s->config->n_iters,
                0
            };
        (*loop_stack_pos)++;*/
    }

    bool continue_loop = loop_stack[*loop_stack_pos].index < *for_s->config->n_iters;
    if(!continue_loop) {
        memset(exclude_list + queue_pos, 1, loop_stack[*loop_stack_pos].body_len + 2);
        //for_s->config->loop_in_process = false;
        //(*loop_stack_pos)--;
        return;
    }
    //memset(exclude_list + queue_pos, 0, loop_stack[*loop_stack_pos - 1].body_len + 2);
    //loop_stack[*loop_stack_pos - 1].offset = loop_stack[*loop_stack_pos - 1].index
    //    ? loop_stack[*loop_stack_pos - 1].offset + 1
    //    : loop_stack[*loop_stack_pos - 1].offset + 1; //update_loop_body_vector_variables(loop_stack, *loop_stack_pos);
}


static inline void
hotcall_handle_while_begin(struct hotcall_while_start *while_s, struct loop_stack_item *loop_stack, unsigned int *loop_stack_pos, uint8_t *exclude_list, int queue_pos, struct ecall_queue_item **queue) {

    /*if(!while_s->config->loop_in_process) {
        while_s->config->loop_in_process = true;
        loop_stack[*loop_stack_pos].body_len = while_s->config->body_len;
        loop_stack[*loop_stack_pos].index = 0;
        if(*loop_stack_pos > 0) {
            loop_stack[*loop_stack_pos].offset = loop_stack[*loop_stack_pos - 1].offset;
        }
        (*loop_stack_pos)++;
    }

    struct postfix_item output[strlen(while_s->config->predicate_fmt)];
    unsigned int output_length;
    //output_length = to_postfix(while_s->config->predicate_fmt, while_s->params, output);
    int res = evaluate_postfix(tif->config->postfix, tif->config->postfix_length, hotcall_config, loop_stack[*loop_stack_pos - 1].offset);
    if(res) {
        memset(exclude_list + queue_pos, 0, loop_stack[*loop_stack_pos - 1].body_len + 2);
        return;
    }
    while_s->config->loop_in_process = false;
    memset(exclude_list + queue_pos, 1, loop_stack[*loop_stack_pos - 1].body_len + 2);
    --(*loop_stack_pos);*/
}

static inline unsigned int
hotcall_end_loop(struct loop_stack_item *loop_stack, unsigned int loop_stack_pos, int *pos) {
    *pos = *pos - (loop_stack[loop_stack_pos].body_len + 2);
    loop_stack[loop_stack_pos].index++;
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
    //ptr->dst = &ptr->src;
    //ptr->dst.offset = ptr->src.offset;
    *(void **) ptr->dst->value.pointer.arg = parse_argument(ptr->src, ptr->offset);
    //printf("ptr: %p offset %d\n", *(void **) ptr->dst->value.pointer.arg, ptr->offset);
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
            unsigned int current_offset = 0;


            unsigned int queue_length = sm_ctx->hcall.batch.queue_len;

            int addr_mod_list[10];
            int n = 0;
            while(n < queue_length) {
            //for(n = 0; n < queue_length; ++n) {
                queue_item = sm_ctx->hcall.batch.queue[n];
                if(exclude_list[n]) {
                    n++;
                    continue;
                }

                //assert(queue_item->type == QUEUE_ITEM_TYPE_FUNCTION);

                switch(queue_item->type) {
                    case QUEUE_ITEM_TYPE_DESTROY:
                        goto exit;
                    case QUEUE_ITEM_TYPE_FUNCTION:
                        fc = &queue_item->call.fc;
                        parse_function_arguments(fc->params, fc->config->n_params, (loop_stack_pos > 0 ? loop_stack[loop_stack_pos - 1].offset : 0), fc->args);
                        hotcall_config->execute_function(fc->config->function_id, fc->args, fc->return_value);
                        break;
                    case QUEUE_ITEM_TYPE_IF: case QUEUE_ITEM_TYPE_IF_NULL:
                        hotcall_handle_if(&queue_item->call.tif, exclude_list, n, queue_length, loop_stack_pos > 0 ? loop_stack[loop_stack_pos - 1].offset : 0);
                        break;
                    case QUEUE_ITEM_TYPE_FOR_EACH:
                        hotcall_handle_for_each(&queue_item->call.tor);
                        break;
                    case QUEUE_ITEM_TYPE_FOR_BEGIN:
                        if(*queue_item->call.for_s.config->n_iters == 0) {
                            memset(exclude_list + n, 1, queue_item->call.for_s.config->body_len + 2);
                        } else {
                            loop_stack[loop_stack_pos] = (struct loop_stack_item) {
                                .body_len = queue_item->call.for_s.config->body_len,
                                .index = 0,
                                .n_iters = *queue_item->call.for_s.config->n_iters,
                            };
                            loop_stack[loop_stack_pos].offset = update_loop_body_vector_variables(loop_stack, loop_stack_pos);
                            loop_stack_pos++;
                        }
                        break;
                    case QUEUE_ITEM_TYPE_FILTER:
                        hotcall_handle_filter(&queue_item->call.fi);
                        break;
                    case QUEUE_ITEM_TYPE_DO_WHILE:
                        hotcall_handle_do_while(&queue_item->call.dw);
                        break;
                    case QUEUE_ITEM_TYPE_FOR_END:
                        if(++loop_stack[loop_stack_pos - 1].index < loop_stack[loop_stack_pos - 1].n_iters) {
                            n = n - (loop_stack[loop_stack_pos - 1].body_len);
                            memset(exclude_list + n, 0, loop_stack[loop_stack_pos - 1].body_len);
                            loop_stack[loop_stack_pos - 1].offset++;
                            goto continue_loop;
                        } else {
                            loop_stack_pos--;
                        }
                        break;
                    case QUEUE_ITEM_TYPE_WHILE_BEGIN:
                        if(!evaluate_postfix(queue_item->call.while_s.config->postfix, queue_item->call.while_s.config->postfix_length, hotcall_config, (loop_stack_pos > 0 ? loop_stack[loop_stack_pos - 1].offset : 0 ))) {
                            memset(exclude_list + n, 1, queue_item->call.while_s.config->body_len + 2);
                            loop_stack_pos--;
                            queue_item->call.while_s.config->loop_in_process = false;
                            break;
                        }
                        if(!queue_item->call.while_s.config->loop_in_process) {
                            queue_item->call.while_s.config->loop_in_process = true;
                            loop_stack[loop_stack_pos].body_len =  queue_item->call.while_s.config->body_len;
                            loop_stack[loop_stack_pos].offset = (loop_stack_pos > 0) ? loop_stack[loop_stack_pos - 1].offset : 0;
                            loop_stack_pos++;
                        } else {
                            memset(exclude_list + n, 0, loop_stack[loop_stack_pos - 1].body_len + 2);
                        }
                        break;
                    case QUEUE_ITEM_TYPE_WHILE_END:
                        n = n - (loop_stack[loop_stack_pos - 1].body_len + 1);
                        goto continue_loop;
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
                n++;
                continue_loop: ;
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
