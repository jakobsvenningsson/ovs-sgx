#include "benchmark.h"
#include "hotcall-bundler-untrusted.h"
#include "functions.h"
/*
unsigned int
benchmark_filter_naive(struct shared_memory_ctx *sm_ctx, unsigned int n_rounds) {
    unsigned int warmup = n_rounds / 10;
    unsigned int rounds[n_rounds];
    unsigned int n_iters = 10, n_params = 1;
    for(int i = 0; i < (n_rounds + warmup); ++i) {
        clear_cache();
        BEGIN

        unsigned int xs[n_iters] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        int ys[n_iters] = { 0 };
        unsigned int out_length = 0;

        void *args[n_params];
        for(int j = 0; j < n_iters; ++j) {
            bool res;
            args[0] =  xs + j ;
            HCALL(sm_ctx, ecall_greater_than_two, false, &res, 1, args);
            if(res) {
                ys[out_length++] = xs[j];
            }
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
benchmark_filter_optimized(struct shared_memory_ctx *sm_ctx, unsigned int n_rounds) {
    unsigned int warmup = n_rounds / 10;
    unsigned int rounds[n_rounds];
    for(int i = 0; i < (n_rounds + warmup); ++i) {
        clear_cache();
        BEGIN

        hotcall_bundle_begin(sm_ctx);
        unsigned int n_params = 1, n_iters = 10, out_length;

        int xs[n_iters] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        int ys[n_iters] = { 0 };

        struct function_parameter function_params_in[n_params] = {
            (struct function_parameter) { .arg = xs, .fmt = 'd', .iter = true }
        };
        struct function_parameters_in params_in = {
            .params = function_params_in, .n_params = n_params, .iters = &n_iters
        };

        struct function_parameter function_params_out[n_params] = {
            (struct function_parameter) { .arg = ys, .fmt = 'd', .iter = true }
        };
        struct function_parameters_in params_out = {
            .params = function_params_out, .n_params = n_params, .iters = &out_length
        };

        struct hotcall_function fc = {
            .id = hotcall_ecall_greater_than_two
        };
        unsigned int n_variables = 1;
        char fmt[] = "b";
        struct predicate_variable variables[n_variables] = {
            (struct predicate_variable) { &fc, FUNCTION_TYPE, 'b' }
        };

        FILTER(
            sm_ctx,
            ((struct filter_args) {
                .params_in = &params_in,
                .params_out = &params_out,
                .predicate = (struct predicate)  {
                    .fmt = fmt,
                    .n_variables = n_variables,
                    .variables = variables
                }
            })
        );

        hotcall_bundle_end(sm_ctx);

        CLOSE
        if(i >= warmup) {
            rounds[i - warmup] = GET_TIME
        }
    }
    qsort(rounds, n_rounds, sizeof(unsigned int), cmpfunc);
    return rounds[n_rounds / 2];
}*/
