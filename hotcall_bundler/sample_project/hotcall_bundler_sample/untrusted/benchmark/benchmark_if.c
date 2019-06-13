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
        //HCALL(sm_ctx, ecall_always_true, false, &res, 0, NULL);
        HCALL(
                ((struct hotcall_function_config) { .function_id = hotcall_ecall_always_true, .has_return = true }),
		        (struct parameter) { .type = VARIABLE_TYPE_, .value = { .variable = { .arg = &res }}}
        );
        if(res) {
            //HCALL(sm_ctx, ecall_foo, false, NULL, 0, NULL);
            HCALL(((struct hotcall_function_config) { .function_id = hotcall_ecall_foo, .has_return = false }));
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
        //bool res;
        //HCALL(sm_ctx, ecall_always_true, false, &res, 0, NULL);
        IF(
            ((struct if_config) {
                .predicate_fmt = "b",
                .return_if_false = false
            }),
            (struct parameter) { .type = FUNCTION_TYPE_,  .value = { .function = { .function_id = hotcall_ecall_always_true, .params =  NULL }}}
            //(struct parameter) { .type = VARIABLE_TYPE_,  .value = { .variable = { .arg = &res, .fmt = 'b' }}}
        );
        THEN
            HCALL(
                    ((struct hotcall_function_config) { .function_id = hotcall_ecall_foo, .has_return = false })
                );

        hotcall_bundle_end(sm_ctx);

        CLOSE
        if(i >= warmup) {
            rounds[i - warmup] = GET_TIME
        }
    }
    qsort(rounds, n_rounds, sizeof(unsigned int), cmpfunc);
    return rounds[n_rounds / 2];
}
