#include "hotcall-trusted.h"
#include "enclave-utils.h"
#include "cache-trusted.h"
#include <sgx_thread.h>

#include "lib22.h"

#define INIT_CACHE_VALIDATION_VALUE 99999999;

#ifdef TIMEOUT
#define INIT_TIMER_VALUE 9999999;
static unsigned int timeout_counter = INIT_TIMER_VALUE;
#endif

int
ecall_start_poller(struct shared_memory_ctx *sm_ctx){

    //sgx_thread_mutex_init(&sm_ctx->hcall.mutex, NULL);
    char buf[128];

    unsigned int cache_validation_timeout = INIT_CACHE_VALIDATION_VALUE;

    while (1) {
        /*if(--cache_validation_timeout == 0) {
            cache_validation_timeout = INIT_CACHE_VALIDATION_VALUE;
            if(!flow_map_cache_is_valid(&sm_ctx->flow_cache)) {
                printf("VARNING: Flow cache hash is incorrect.\nFlushing cache and reporting the incident.\n");
                flow_map_cache_flush(&sm_ctx->flow_cache);
            } else {
                //printf("FLOW CACHE IS VALID!\n");
            }
        }*/
        sgx_spin_lock(&sm_ctx->hcall.spinlock);

        if (sm_ctx->hcall.run) {

            #ifdef TIMEOUT
            timeout_counter = INIT_TIMER_VALUE;
            #endif
            sm_ctx->hcall.run = false;
            struct ecall_queue_item *queue_item, *queue_item_next;
            bool discard_rest = false, execute_else_branch = false, discard_next = false;


            int sz = list_size(&sm_ctx->hcall.ecall_queue);
            uint8_t exclude_list[sz];
            for(int i = 0; i < sz; ++i) exclude_list[i] = 0;

            struct function_call *fc, *prev_fc;
            struct transaction_assert *ta;
            struct transaction_if *tif;
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
                        execute_function(fc, &sm_ctx->flow_cache);
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


                        /*int i = 0;
                        while(tif->n_conditions > 0) {
                            switch(tif->comparators[i - 1]) {
                                case 'o':
                                    outcome = outcome || *(int *) tif->conditions[i];
                                    switch(tif->type[0]) {
                                        case 's':
                                            outcome = outcome || *(int *) tif->conditions[0];
                                            break;
                                        case 'g':
                                            outcome = *(int *) tif->conditions[0] > *(int *) tif->conditions[1];
                                            break;
                                        default:
                                            printf("unknown type.\n");
                                    }
                                    break;
                                case 'a':
                                    outcome = outcome && *(int *) tif->conditions[i];
                                    break;
                                default:
                                    printf("unknown condition");
                            }
                        }
                        for(int i = 1; i < tif->n_conditions; ++i) {

                        }

                            int index = 0;
                            while(tif->n_controllers > 0) {
                                switch(tif->comparators[index]) {
                                        case 'o':
                                            outcome
                                            break;
                                }
                            }
                            for(int i = 0; i < tif->n_conditions - 1; ++i) {
                                switch(tif->comparators[i]) {
                                    case "g":

                                        outcome = outcome &
                                        break;
                                    default:
                                        printf("unknown comparator.\n");
                                }
                            }*/
                        //}
                        break;
                    default:
                        printf("Error, the default statement should never happen...\n");
                }
                list_remove(&queue_item->list_node);

                /*fcall = &queue_item->call.fc;
                if(discard_rest || fcall->id == hotcall_transaction_guard) {
                    list_remove(&queue_item->list_node);
                    continue;
                }*/
                /*if(fcall->id == hotcall_transaction_assert_false_) {
                    // The condition is true
                    if(*(int *) next->args.args[0] == 0) {
                        // Check if there existed a assert handler and throw it awway in this case
                        if(*(uint8_t *) next->args.args[1] == 1) {
                            list_remove(&next->list_node);
                        }
                    } else {
                        discard_rest = true;
                        if(*(uint8_t *) next->args.args[1] == 1) {
                            printf("Executing cleanup function: %d.\n", next->id);
                            execute_function(next, &sm_ctx->flow_cache);
                        }
                    }
                    list_remove(&fcall->list_node);
                    continue;
                }*/
                //printf("Executing function: %d.\n", fcall->id);
                //execute_function(fcall, &sm_ctx->flow_cache);
                /*if(sm_ctx->hcall.transaction_in_progress && next->id == hotcall_transaction_guard) {
                    int expected = *(int *) next->args.args[0];
                    if(*(int *) fcall->return_value != expected) {
                        discard_rest = true;
                    }
                }*/
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

        //printf("%d\n", timeout_counter);

        #ifdef TIMEOUT
        timeout_counter--;
        if (timeout_counter <= 0) {
            printf("HOTCALL SERVICE SLEEPING.\n");
            timeout_counter = INIT_TIMER_VALUE;
            sm_ctx->hcall.sleeping   = true;
            ocall_sleep();
            sm_ctx->hcall.sleeping = false;
        }
        #endif /* ifdef TIMEOUT */
    }


    sgx_spin_unlock(&sm_ctx->hcall.spinlock);

    return 0;
} /* ecall_start_poller */
