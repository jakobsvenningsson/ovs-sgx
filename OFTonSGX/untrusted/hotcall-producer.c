#include "hotcall-producer.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "sgx-utils.h"


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond   = PTHREAD_COND_INITIALIZER;
static async_ecall ctx;

void
make_hotcall(async_ecall * ctx, int function, argument_list * args, void * ret){

    ctx->function = function;
    ctx->args     = args;
    ctx->ret      = ret;
    ctx->is_done  = false;
    ctx->run      = true;

    while (1) {
        #ifdef TIMEOUT
        if (ctx->sleeping) {
            pthread_mutex_lock(&mutex);
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&mutex);
            continue;
        }
        #endif

        sgx_spin_lock(&ctx->spinlock);
        if (ctx->is_done) {
            sgx_spin_unlock(&ctx->spinlock);
            break;
        }
        sgx_spin_unlock(&ctx->spinlock);
        for (int i = 0; i < 3; ++i) {
            __asm
            __volatile(
              "pause"
            );
        }
    }
}

void compile_arg_list(void **return_val, argument_list *arg_list, bool has_return, int n_args, ...) {
    arg_list->n_args = n_args;
    va_list args;
    va_start(args, n_args);

    if(has_return) {
        *return_val = va_arg(args, void *);
    }
    for(int i = 0; i < n_args; ++i) {
        arg_list->args[i] = va_arg(args, void *);
    }
    va_end(args);
}


/* void cleanup_hotcall(char *fmt, argument_list * arg_list) {
    int n_args = arg_list->n_args;
    if(*fmt == 'r') {
        fmt++;
        n_args--;
    }
    for(int i = 0; i < n_args; ++i) {
        switch(fmt[i]) {
            case 'p':
                 break;
            case 'd':
            {
                free((int *) arg_list->args[i]);
                break;
            }
            case 'u':
            {
                free((unsigned int *) arg_list->args[i]);
                break;
            }
            case 't':
            {
                free((size_t *) arg_list->args[i]);
                break;
            }
            case 'o':
            {
                free((uint32_t *) arg_list->args[i]);
                break;
            }
            case 'b':
            {
                free((bool *) arg_list->args[i]);
                break;
            }
            case 'q':
            {
                free((int *) arg_list->args[i]);
                break;
            }
            default:
                printf("Cleanup: unknown character in format string %c.\n", fmt[i]);
        }
    }
} */


void
ocall_sleep(){
    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);
}
