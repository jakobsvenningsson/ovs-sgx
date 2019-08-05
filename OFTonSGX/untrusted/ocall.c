#include "ocall.h"
#include <stdio.h>      /* vsnprintf */
#include <unistd.h>


void ocall_print(const char *str) {
    printf("%s", str);
}


void
ocall_sleep(){
    /*pthread_mutex_lock(&mutex);
    pthread_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);*/
}
