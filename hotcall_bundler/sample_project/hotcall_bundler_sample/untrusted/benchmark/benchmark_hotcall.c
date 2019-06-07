#include "benchmark.h"
#include "hotcall-untrusted.h"
#include "functions.h"
#include <stdarg.h>

unsigned int
benchmark_hotcall(struct shared_memory_ctx *sm_ctx, unsigned int n_rounds) {
    unsigned int warmup = n_rounds / 10;
    unsigned int rounds[n_rounds];
    for(int i = 0; i < (n_rounds + warmup); ++i) {
        clear_cache();
        BEGIN
        HCALL(sm_ctx, ecall_foo, false, NULL, 0, NULL)
        CLOSE
        if(i >= warmup) {
            rounds[i - warmup] = GET_TIME
        }
    }
    qsort(rounds, n_rounds, sizeof(unsigned int), cmpfunc);
    return rounds[n_rounds / 2];
}


void
variadic_function(int n, ...) {
    va_list args;
    va_start(args, n);

    int x = 0;
    for(int i = 0; i < n; ++i) {
        x += va_arg(args, int);
    }

    va_end(args);
}

unsigned int
benchmark_variadic_function(struct shared_memory_ctx *sm_ctx, unsigned int n_rounds) {
    unsigned int warmup = n_rounds / 10;
    unsigned int rounds[n_rounds];
    for(int i = 0; i < (n_rounds + warmup); ++i) {
        clear_cache();
        BEGIN
        variadic_function(10, 1, 2, 3, 4, 5, 6, 7, 8, 9);
        CLOSE
        if(i >= warmup) {
            rounds[i - warmup] = GET_TIME
        }
    }
    qsort(rounds, n_rounds, sizeof(unsigned int), cmpfunc);
    return rounds[n_rounds / 2];
}
