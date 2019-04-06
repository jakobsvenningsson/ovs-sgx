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
    ctx->run      = true;
    ctx->is_done  = false;

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

argument_list * compile_arg_list(char *fmt, void **ret, int n_args, ...) {
    argument_list *arg_list = malloc(sizeof(argument_list));
    arg_list->n_args = n_args;
    va_list args;
    va_start(args, n_args);
    int i = 0;
    while(*fmt) {
        switch((*fmt)) {
            case 'r':
            {
                void  *r = va_arg(args, void *);
                *ret = r;
                fmt++;
                continue;
            }
            case 'p':
            {
                void  *p = va_arg(args, void *);
                arg_list->args[i] = p;
                break;
            }
            case 'd':
            {
                int *d = malloc(sizeof(int));
                *d = va_arg(args, int);
                arg_list->args[i] = (void *) d;
                break;
            }
            case 'u':
            {
                unsigned int *u = malloc(sizeof(unsigned int));
                *u = va_arg(args, unsigned int);
                arg_list->args[i] = (void *) u;
                break;
            }
            case 't':
            {
                size_t *t = malloc(sizeof(size_t));
                arg_list->args[i] = (void *) t;
                break;
            }
            case 'o':
            {
                uint32_t *o = malloc(sizeof(uint32_t));
                *o = va_arg(args, uint32_t);
                arg_list->args[i] = (void *) o;
                break;
            }
            case 'b':
            {
                bool *b = malloc(sizeof(bool));
                *b = va_arg(args, bool);
                arg_list->args[i] = (void *) b;
                break;
            }
            case 'q':
            {
                int *q = malloc(sizeof(int));
                *q = va_arg(args, int);
                arg_list->args[i] = (void *) q;
                break;
            }
            default:
                printf("unknown character in format string %c.\n", *fmt);
                return NULL;
        }
        i++;
        fmt++;
    }
    return arg_list;
}


void
ocall_sleep(){
    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);
}
