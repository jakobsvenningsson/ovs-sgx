#include "hotcall_bundler_t.h"  /* print_string */
#include "hotcall-trusted.h"
/*
// Its recommended by intel to add pause actions inside spinlock loops in order to increase performance.
for (int i = 0; i < 3; ++i) {
    __asm
    __volatile(
      "pause"
    );
}
printf("after\n");
for(int i = 0; i < hotcall_config->n_spinlock_jobs; ++i) {
    printf("inside\n");
    if(hotcall_config->spin_lock_task_count[i] == spin_lock_task_timeouts[i]) {
            hotcall_config->spin_lock_tasks[i]();
            hotcall_config->spin_lock_task_count[i] = 0;
            continue;
    }
    hotcall_config->spin_lock_task_count[i]++;
}

*/

static struct hotcall_config *hotcall_config;

void
hotcall_register_config(struct hotcall_config *config) {
    hotcall_config = config;
}

int
ecall_start_poller(struct shared_memory_ctx *sm_ctx){


    struct ecall_queue_item *queue_item, *queue_item_next;
    struct function_call *fc, *prev_fc;
    struct transaction_assert *ta;
    struct transaction_if *tif;

    while (1) {

        sgx_spin_lock(&sm_ctx->hcall.spinlock);

        if (sm_ctx->hcall.run) {

            #ifdef TIMEOUT
            timeout_counter = INIT_TIMER_VALUE;
            #endif
            sm_ctx->hcall.run = false;
            bool discard_rest = false, execute_else_branch = false, discard_next = false;

            int sz = list_size(&sm_ctx->hcall.ecall_queue);
            uint8_t exclude_list[sz];
            for(int i = 0; i < sz; ++i) exclude_list[i] = 0;

            int n = 0;
            LIST_FOR_EACH_SAFE(queue_item, queue_item_next, list_node, &sm_ctx->hcall.ecall_queue) {
                    if(exclude_list[n++]) {
                        list_remove(&queue_item->list_node);
                        continue;
                    }
                    switch(queue_item->type) {
                        case 3:
                            break;
                        case 0:
                            fc = &queue_item->call.fc;
                            printf("Execting function %d.\n", fc->id);
                            hotcall_config->execute_function(fc);
                            prev_fc = fc;
                            break;
                        case 1:
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
                        case 2:
                            tif = &queue_item->call.tif;
                            bool outcome;
                            switch(tif->predicate_type) {
                                case 0:
                                    if(tif->predicate.null_type.condition == NULL) {
                                        memset(exclude_list + n + tif->if_len, 1, tif->else_len);
                                    } else {
                                        memset(exclude_list + n, 1, tif->if_len);
                                    }
                                    break;
                                case 1:
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

                                    if(outcome == tif->predicate.num_type.expected) {
                                        memset(exclude_list + n + tif->if_len, 1, tif->else_len);
                                    } else {
                                        memset(exclude_list + n, 1, tif->if_len);
                                    }
                                    break;
                                default:
                                    break;
                            }
                            break;
                        default:
                            printf("Error, the default statement should never happen...\n");
                    }
                    list_remove(&queue_item->list_node);
                }
                sm_ctx->hcall.is_done = true;
                sgx_spin_unlock(&sm_ctx->hcall.spinlock);
                continue;
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

    sgx_spin_unlock(&sm_ctx->hcall.spinlock);


    return 0;
} /* ecall_start_poller */
