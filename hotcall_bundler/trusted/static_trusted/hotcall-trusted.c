#include "hotcall_bundler_t.h"  /* print_string */
#include "hotcall-trusted.h"

static struct hotcall_config *hotcall_config;

void
hotcall_register_config(struct hotcall_config *config) {
    hotcall_config = config;
}

#define PREDICATE_TYPE_NULL 0
#define PREDICATE_TYPE_CLAUSES 1


inline void
exclude_else_branch(uint8_t *exclude_list, int pos, unsigned int if_len, unsigned int else_len) {
    memset(exclude_list + pos + if_len + 1, 1, else_len);
}

inline void
exclude_if_branch(uint8_t *exclude_list, int pos, unsigned int if_len) {
    memset(exclude_list + pos + 1, 1, if_len);
}

inline void
hotcall_handle_if(struct transaction_if *tif, uint8_t *exclude_list, int pos) {
    bool outcome;
    unsigned int else_len = *tif->else_len, if_len = *tif->if_len;

    switch(tif->predicate_type) {
        case PREDICATE_TYPE_NULL:
            if(tif->predicate.null_type.condition == NULL && else_len > 0) {
                exclude_else_branch(exclude_list, pos, if_len, else_len);
            } else if(tif->if_len > 0) {
                exclude_if_branch(exclude_list, pos, if_len);
            }
            break;
        case PREDICATE_TYPE_CLAUSES:
            switch(tif->predicate.num_type.type[0]) {
                case 's':
                    outcome = *(unsigned int *) tif->predicate.num_type.conditions[0];
                    break;
                case '>':
                    outcome = *(unsigned int *) tif->predicate.num_type.conditions[0] > *(unsigned int *) tif->predicate.num_type.conditions[1];
                    break;
                default:
                    printf("unknown type.\n");
            }
            if(outcome == *tif->predicate.num_type.expected  && else_len > 0) {
                exclude_else_branch(exclude_list, pos, if_len, else_len);
            } else if(outcome != *tif->predicate.num_type.expected && tif->if_len > 0) {
                exclude_if_branch(exclude_list, pos, if_len);
            }
            break;
        default:
            printf("Unknown predicate type.\n");
    }
}

int
ecall_start_poller(struct shared_memory_ctx *sm_ctx){

    printf("Starting shared memory poller.\n");

    struct ecall_queue_item *queue_item, *queue_item_next;
    struct function_call *fc, *prev_fc;
    struct transaction_assert *ta;
    struct transaction_if *tif;

    while (1) {

        sgx_spin_lock(&sm_ctx->hcall.spinlock);

        if (sm_ctx->hcall.run) {

            sm_ctx->hcall.run = false;

            uint8_t exclude_list[sm_ctx->hcall.queue_length];
            memset(exclude_list, 0, sm_ctx->hcall.queue_length);

            for(int n = 0; n < sm_ctx->hcall.queue_length; ++n) {
                    if(exclude_list[n]) {
                        continue;
                    }
                    queue_item = sm_ctx->hcall.ecall_queue[n];

                    switch(queue_item->type) {
                        case QUEUE_ITEM_TYPE_DESTROY:
                            goto exit;
                        case QUEUE_ITEM_TYPE_FUNCTION:
                            fc = &queue_item->call.fc;
                            hotcall_config->execute_function(fc);
                            prev_fc = fc;
                            break;
                        case QUEUE_ITEM_TYPE_GUARD:
                            if(!prev_fc) {
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
                            }
                            break;
                        case QUEUE_ITEM_TYPE_IF: case QUEUE_ITEM_TYPE_IF_NULL:
                            tif = &queue_item->call.tif;
                            hotcall_handle_if(&queue_item->call.tif, exclude_list, n);
                            break;
                        default:
                            printf("Error, the default statement should never happen...\n");
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
