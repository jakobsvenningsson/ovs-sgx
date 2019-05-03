#include "hotcall-trusted.h"
#include "enclave-utils.h"
#include "cache-trusted.h"

#define INIT_CACHE_VALIDATION_VALUE 99999999;

#ifdef TIMEOUT
#define INIT_TIMER_VALUE 9999999;
static unsigned int timeout_counter = INIT_TIMER_VALUE;
#endif

int
ecall_start_poller(async_ecall * ctx){
    // sgx_thread_mutex_init(&ctx->mutex, NULL);
    char buf[128];

    unsigned int cache_validation_timeout = INIT_CACHE_VALIDATION_VALUE;

    while (1) {
        if(--cache_validation_timeout == 0) {
            cache_validation_timeout = INIT_CACHE_VALIDATION_VALUE;
            if(!flow_map_cache_is_valid(&ctx->flow_cache)) {
                printf("VARNING: Flow cache hash is incorrect.\nFlushing cache and reporting the incident.\n");
                flow_map_cache_flush(&ctx->flow_cache);
            } else {
                printf("FLOW CACHE IS VALID!\n");
            }
        }

        sgx_spin_lock(&ctx->spinlock);

        if (ctx->run) {
            #ifdef TIMEOUT
            timeout_counter = INIT_TIMER_VALUE;
            #endif
            ctx->run = false;
            execute_function(ctx->function, ctx->args, ctx->ret, &ctx->flow_cache);
            ctx->is_done = true;
            sgx_spin_unlock(&ctx->spinlock);
            continue;
        }

        sgx_spin_unlock(&ctx->spinlock);

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
            ctx->sleeping   = true;
            ocall_sleep();
            ctx->sleeping = false;
        }
        #endif /* ifdef TIMEOUT */
    }


    sgx_spin_unlock(&ctx->spinlock);

    return 0;
} /* ecall_start_poller */
