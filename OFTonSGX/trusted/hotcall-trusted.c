#include "hotcall-trusted.h"
#include "enclave-utils.h"
#include "cache-trusted.h"

#define INIT_CACHE_VALIDATION_VALUE 99999999;

#ifdef TIMEOUT
#define INIT_TIMER_VALUE 9999999;
static unsigned int timeout_counter = INIT_TIMER_VALUE;
#endif

int
ecall_start_poller(struct shared_memory_ctx *sm_ctx){
    // sgx_thread_mutex_init(&ctx->mutex, NULL);
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
            struct function_call *fcall, *next;
            LIST_FOR_EACH_SAFE(fcall, next, list_node, &sm_ctx->hcall.ecall_queue) {
                execute_function(fcall, &sm_ctx->flow_cache);
                list_remove(&fcall->list_node);
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
