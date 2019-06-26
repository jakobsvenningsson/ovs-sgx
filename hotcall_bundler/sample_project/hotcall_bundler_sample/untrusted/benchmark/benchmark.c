#include "benchmark.h"
#include <stdio.h>

void
benchmark(struct shared_memory_ctx *sm_ctx, unsigned int (*f_benchmark)(struct shared_memory_ctx *, unsigned int), unsigned int n_rounds, unsigned int n_iters) {
    unsigned int r, avg_median = 0;
    for(int n = 0; n < n_iters; ++n) {
        printf("Running iteration %d.\n", n);
        r = f_benchmark(sm_ctx, n_rounds);
        printf("%d\n", r);
        avg_median += r;
    }
    printf("Median %u\n", avg_median/n_iters);
}

int
cmpfunc (const void * a, const void * b) {
   return ( *(unsigned int*)a - *(unsigned int*)b );
}
 
