#include "benchmark.h"
#include "hotcall-untrusted.h"
#include "functions.h"

unsigned int
benchmark_if_naive(struct shared_memory_ctx *sm_ctx, unsigned int n_rounds) {
    unsigned int warmup = n_rounds / 10;
    unsigned int rounds[n_rounds];
    for(int i = 0; i < (n_rounds + warmup); ++i) {
        clear_cache();
        BEGIN
        bool res;
        HCALL(sm_ctx, ecall_always_true, false, &res, 0, NULL);
        if(res) {
            HCALL(sm_ctx, ecall_foo, false, NULL, 0, NULL)
        }
        CLOSE
        if(i >= warmup) {
            rounds[i - warmup] = GET_TIME
        }
    }
    qsort(rounds, n_rounds, sizeof(unsigned int), cmpfunc);
    return rounds[n_rounds / 2];
}


unsigned int
benchmark_if_optimized(struct shared_memory_ctx *sm_ctx, unsigned int n_rounds) {
    unsigned int warmup = n_rounds / 10;
    unsigned int rounds[n_rounds];
    for(int i = 0; i < (n_rounds + warmup); ++i) {
        clear_cache();
        BEGIN
        hotcall_bundle_begin(sm_ctx);
        bool res;
        HCALL(sm_ctx, ecall_always_true, false, &res, 0, NULL);
        int n_variables = 1;
        struct predicate_variable variables[n_variables] = {
            (struct predicate_variable) { &res, VARIABLE_TYPE, 'b' }
        };
        char fmt[] = "b";
        struct if_args if_args = {
            .then_branch_len = 1,
            .else_branch_len = 0,
            .predicate = (struct predicate) {
                .fmt = fmt,
                .n_variables = n_variables,
                .variables = variables
            },
            .return_if_false = false
        };
        IF(
            sm_ctx,
            &if_args
        );
        THEN(HCALL(sm_ctx, ecall_foo, false, NULL, 0, NULL));
        ELSE(NULL);

        hotcall_bundle_end(sm_ctx);
        CLOSE
        if(i >= warmup) {
            rounds[i - warmup] = GET_TIME
        }
    }
    qsort(rounds, n_rounds, sizeof(unsigned int), cmpfunc);
    return rounds[n_rounds / 2];
}
