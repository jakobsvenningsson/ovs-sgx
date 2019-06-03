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

#define PREDICATE_TYPE_NULL 0
#define PREDICATE_TYPE_CLAUSES 1


static inline void
exclude_else_branch(uint8_t *exclude_list, int pos, unsigned int if_len, unsigned int else_len) {
    memset(exclude_list + pos + if_len + 1, 1, else_len);
}

static inline void
exclude_if_branch(uint8_t *exclude_list, int pos, unsigned int if_len) {
    memset(exclude_list + pos + 1, 1, if_len);
}

static inline void
exclude_rest(uint8_t *exclude_list, int pos, int then_branch_len, int else_branch_len, int len) {
    int exclude_start = pos + then_branch_len + else_branch_len + 1;
    memset(exclude_list + exclude_start, 1, len - exclude_start);
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

    /*bool predicate_res = true, clause = true;
    for(int i = 0; i < tif->predicate.n_clauses; ++i) {
        switch(tif->predicate.clause_types[i]) {
            case FUNCTION_TYPE:
            {
                struct function_call *fc;
                fc = tif->predicate.clauses[i].fc;
                switch(tif->predicate.fmt[i]) {
                    case 'b':
                        fc->return_value = &clause;
                        break;
                    default:
                        printf("unknown type.\n");
                }
                hotcall_config->execute_function(fc);
                break;

            }

            case POINTER_TYPE:
                clause = (void *) tif->predicate.clauses[i].ptr == NULL;
                break;
            case VARIABLE_TYPE_SINGLE:
                switch(tif->predicate.fmt[i]) {
                    case 'b':
                        clause = *(bool *) tif->predicate.clauses[i].var;
                        break;
                    default:
                        printf("unknown type.\n");
                }
                break;
            default:
                printf("unknown cluase type %s %s.\n", __FILE__, __LINE__);
                break;
        }

        if(i == 0) {
            predicate_res = clause;
            continue;
        }
        switch(tif->predicate.connections[i - 1]) {
            case CONJUNCTION:
                predicate_res = predicate_res && clause;
            case DISJUNCTION:
                predicate_res = predicate_res || clause;
                break;
            default:
                printf("unknown connection type %s %s\n", __FILE__, __LINE__);
        }
    }

    if(predicate_res == tif->predicate.expected && tif->args.else_branch_len > 0) {
        exclude_else_branch(exclude_list, pos, tif->args.then_branch_len, tif->args.else_branch_len);
    } else if(predicate_res != tif->predicate.expected && tif->args.then_branch_len > 0) {
        exclude_if_branch(exclude_list, pos, tif->args.then_branch_len);
    }*/
}

/*
static inline void
hotcall_handle_if(struct transaction_if *tif, uint8_t *exclude_list, int pos) {
    int outcome;
    unsigned int else_len = tif->args.else_branch_len, if_len = tif->args.then_branch_len;

    switch(tif->predicate_type) {
        case PREDICATE_TYPE_NULL:
            if(tif->predicate.null_type.condition == NULL && else_len > 0) {
                exclude_else_branch(exclude_list, pos, if_len, else_len);
            } else if(tif->predicate.null_type.condition != NULL && tif->args.then_branch_len > 0) {
                exclude_if_branch(exclude_list, pos, if_len);
            }
            break;
        case PREDICATE_TYPE_CLAUSES:
            ;
            bool outcome;
            outcome = translate_expression(&tif->predicate.num_type, 0, 0);
            if(outcome == *tif->predicate.num_type.expected  && else_len > 0) {
                exclude_else_branch(exclude_list, pos, if_len, else_len);
            } else if(outcome != *tif->predicate.num_type.expected && tif->args.then_branch_len > 0) {
                exclude_if_branch(exclude_list, pos, if_len);
            }
            break;
        default:
            printf("Unknown predicate type.\n");
    }
}*/

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

/*
static inline void
hotcall_handle_filter(struct transaction_filter *fi) {
    struct immutable_function_argument *args;
    args = fi->args;
    struct function_call fc;
    fc.id = fi->f;
    fc.args.n_args = args->n_params;
    int n_include = 0;
    for(int i = 0; i < args->params_in_length; ++i) {
        for(int j = 0; j < args->n_params; ++j) {
            switch(args->fmt[j]) {
                case 'd':
                    fc.args.args[j] = (int *) args->params_in[j] + i;
                    break;
                default:
                    break;
            }
        }
        bool include;
        fc.return_value = (void *) &include;
        hotcall_config->execute_function(&fc);
        if(include) {
            for(int j = 0; j < args->n_params; ++j) {
                switch(args->fmt[j]) {
                    case 'd':
                        ((int *) args->params_out[j])[n_include] = ((int *) args->params_in[j])[i];
                        break;
                    default:
                        break;
                }
            }
            n_include++;
        }
    }
    args->params_out_length = n_include;
}*/
/*
static inline void
hotcall_handle_for(struct transaction_for_each *tor) {
    struct immutable_function_argument *args;
    args = tor->args;
    struct function_call fc;
    fc.id = tor->f;
    fc.args.n_args = args->n_params;
    for(int i = 0; i < args->params_in_length; ++i) {
        for(int j = 0; j < args->n_params; ++j) {
            switch(args->fmt[j]) {
                case 'd':
                    fc.args.args[j] = (int *) args->params_in[j] + i;
                    break;
                default:
                    break;
            }
        }
        hotcall_config->execute_function(&fc);
    }
}
*/

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

int
ecall_start_poller(struct shared_memory_ctx *sm_ctx){

    printf("Starting shared memory poller.\n");

    struct ecall_queue_item *queue_item, *queue_item_next;
    struct function_call *fc, *prev_fc;
    struct transaction_assert *ta;
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
                        {
                            fc = &queue_item->call.fc;
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
                                    int *p = ((int *) fc->args.args[i] + for_loop_indices[for_loop_nesting - 1]);
                                    p = (int *) fc->args.args[i];
                                    fc->args.args[0] = tmp[i];
                                }
                            }
                            prev_fc = fc;
                            break;
                        }
                        case QUEUE_ITEM_TYPE_GUARD:
                            /*if(!prev_fc) {
                                break;
                            }
                            ta = &queue_item->call.ta;
                            if(ta->expected != *(int *) prev_fc->return_value) {
                                memset(exclude_list, 1, sizeof(exclude_list)/sizeof(exclude_list[0]));
                                *(ta->transaction_error) = ta->error;
                                if(ta->has_else) {
                                    struct function_call *else_fc;
                                    else_fc = &queue_item_next->call.fc;
                                    printf("Executing else function: %d.\n", else_fc->id);
                                }
                            } else {
                                if(ta->has_else) {
                                    exclude_list[n] = 1;
                                }
                            }*/
                            break;
                        case QUEUE_ITEM_TYPE_IF: case QUEUE_ITEM_TYPE_IF_NULL:
                            hotcall_handle_if(&queue_item->call.tif, exclude_list, n, sm_ctx->hcall.queue_length);
                            break;
                        case QUEUE_ITEM_TYPE_FOR_EACH:
                            hotcall_handle_for(&queue_item->call.tor);
                            break;
                        case QUEUE_ITEM_TYPE_FOR_BEGIN:
                            for_s = &queue_item->call.for_s;
                            if(for_s->n_iters-- > 0) {
                                for_loop_nesting++;
                                break;
                            }
                            for_loop_nesting--;
                            memset(exclude_list + n + 1, 1, for_s->n_rows + 1);
                            break;
                        case QUEUE_ITEM_TYPE_FOR_END:
                            for_e = &queue_item->call.for_e;
                            n = n - (for_e->n_rows + 2);
                            for_loop_nesting--;
                            for_loop_indices[for_loop_nesting]++;
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
