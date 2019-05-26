#include "hotcall-untrusted.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "sgx-utils.h"
#include <assert.h>
#include "enclave_u.h"
#include <string.h>
#include <sched.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond   = PTHREAD_COND_INITIALIZER;
/*
void
make_hotcall(struct hotcall *hcall){

    struct function_call *fcall, *next;
    hcall->is_done  = false;
    hcall->run      = true;

    while (1) {
        #ifdef TIMEOUT
        if (hcall->sleeping) {
            pthread_mutex_lock(&mutex);
            pthread_cond_signal(&cond);
            pthread_mutex_unlock(&mutex);
            continue;
        }
        #endif

        sgx_spin_lock(&hcall->spinlock);
        if (hcall->is_done) {
            sgx_spin_unlock(&hcall->spinlock);
            break;
        }
        sgx_spin_unlock(&hcall->spinlock);
        for (int i = 0; i < 3; ++i) {
            __asm
            __volatile(
              "pause"
            );
        }
    }
}*/
/*
void prepare_hotcall_function(struct hotcall *hcall, uint8_t function_id, char *fmt, bool async, bool has_return, int n_args, ...) {

    struct function_call *f_call = &pfc->fcs[pfc->idx++];
    if(pfc->idx == 20) {
        pfc->idx = 0;
    }
    f_call->id = function_id;
    f_call->async = async;

    f_call->args.n_args = n_args;

    if(async) {
        f_call->fmt = strdup(fmt);
    } else {
        f_call->fmt = fmt;
    }

    va_list args;
    va_start(args, n_args);

    char* token = strtok(fmt, ",");

    if(has_return) {
        f_call->return_value = va_arg(args, void *);
        token = strtok(NULL, ",");
    }

    size_t i = 0;
    while (token) {
        f_call->args.args[i] = va_arg(args, void *);
        i++;
        token = strtok(NULL, ",");
    }

    va_end(args);

    list_push_back(&hcall->ecall_queue, &f_call->list_node);
}*/

void
ocall_sleep(){
    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);
}
