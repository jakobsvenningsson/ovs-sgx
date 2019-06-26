#include "benchmark.h"
#include "hotcall-bundler-untrusted.h"
#include "functions.h"


unsigned int
benchmark_map(struct shared_memory_ctx *sm_ctx, unsigned int n_rounds) {
    unsigned int warmup = n_rounds / 10;
    unsigned int rounds[n_rounds];
    for(int i = 0; i < (n_rounds + warmup); ++i) {
        clear_cache();
        unsigned int n_iters = 10000;
        int xs[n_iters] = { 0 };
        int ys[n_iters] = { 0 };

        BEGIN

        hotcall_bundle_begin(sm_ctx);

        MAP(((struct map_config) {
                .function_id = hotcall_ecall_plus_one,
                .n_iters = &n_iters
            }),
            VECTOR(xs, 'd'),
            VECTOR(ys, 'd')
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

unsigned int
benchmark_for(struct shared_memory_ctx *sm_ctx, unsigned int n_rounds) {
    unsigned int warmup = n_rounds / 10;
    unsigned int rounds[n_rounds];
    for(int i = 0; i < (n_rounds + warmup); ++i) {
        clear_cache();
        unsigned int n_iters = 10000;
        int xs[n_iters] = { 0 };

        BEGIN

        hotcall_bundle_begin(sm_ctx);

        BEGIN_FOR(((struct for_config) {
            .n_iters = &n_iters
        }));

        HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VECTOR(xs, 'd'));

        END_FOR();

        hotcall_bundle_end(sm_ctx);

        CLOSE
        if(i >= warmup) {
            rounds[i - warmup] = GET_TIME
        }
    }
    qsort(rounds, n_rounds, sizeof(unsigned int), cmpfunc);
    return rounds[n_rounds / 2];
}

unsigned int
benchmark_filter(struct shared_memory_ctx *sm_ctx, unsigned int n_rounds) {
    unsigned int warmup = n_rounds / 10;
    unsigned int rounds[n_rounds];


    unsigned int n_iters = 10000;
    int xs[n_iters] = { 0 };
    for(int i = 0; i < n_iters; ++i) {
        xs[i] = (i % 2) ? 3 : 1;
        //xs[i] = i > n_iters / 2 ? 3 : 1;
    }
    int y = 2;
    for(int i = 0; i < (n_rounds + warmup); ++i) {
        clear_cache();

        unsigned int out_length = 0;
        int ys[n_iters] = { 0 };


        BEGIN

        hotcall_bundle_begin(sm_ctx);

        FILTER(
            ((struct filter_config) { .predicate_fmt = "d>d" }),
            VECTOR(xs, 'd', &n_iters), VAR(y, 'd'), VECTOR(ys, 'd', &out_length)
        );


        /*struct parameter function_parameter[] = { VECTOR(xs, 'd', &n_iters) };
        FILTER(((struct filter_config) {
                .predicate_fmt = "b"
            }),
            FUNC(.function_id = hotcall_ecall_greater_than_two, .params = function_parameter, .n_params = 1),
            VECTOR(ys, 'd', &out_length)
        );*/

        hotcall_bundle_end(sm_ctx);

        CLOSE
        if(i >= warmup) {
            rounds[i - warmup] = GET_TIME
        }
    }
    qsort(rounds, n_rounds, sizeof(unsigned int), cmpfunc);
    return rounds[n_rounds / 2];
}


unsigned int
benchmark_for_each(struct shared_memory_ctx *sm_ctx, unsigned int n_rounds) {
    unsigned int warmup = n_rounds / 10;
    unsigned int rounds[n_rounds];


    unsigned int n_iters = 10000;
    int xs[n_iters] = { 0 };

    for(int i = 0; i < (n_rounds + warmup); ++i) {
        clear_cache();

        BEGIN

        hotcall_bundle_begin(sm_ctx);

        FOR_EACH(((struct for_each_config) { .function_id = hotcall_ecall_plus_one, .n_iters = &n_iters}), VECTOR(xs, 'd'));

        hotcall_bundle_end(sm_ctx);

        CLOSE
        if(i >= warmup) {
            rounds[i - warmup] = GET_TIME
        }
    }
    qsort(rounds, n_rounds, sizeof(unsigned int), cmpfunc);
    return rounds[n_rounds / 2];
}

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
                ((struct hotcall_functionconfig) { .function_id = hotcall_ecall_always_true, .has_return = true }),
		        (struct parameter) { .type = VARIABLE_TYPE, .value = { .variable = { .arg = &res }}}
        );
        if(res) {
            //HCALL(sm_ctx, ecall_foo, false, NULL, 0, NULL);
            HCALL(((struct hotcall_functionconfig) { .function_id = hotcall_ecall_foo, .has_return = false }));
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
        HCALL(
                ((struct hotcall_functionconfig) { .function_id = hotcall_ecall_always_true, .has_return = true }),
                (struct parameter) { .type = VARIABLE_TYPE, .value = { .variable = { .arg = &res }}}
        );
        IF(
            ((struct if_config) {
                .predicate_fmt = "b",
                .return_if_false = false
            }),
            //(struct parameter) { .type = FUNCTION_TYPE,  .value = { .function = { .function_id = hotcall_ecall_always_true, .params =  NULL }}}
            (struct parameter) { .type = VARIABLE_TYPE,  .value = { .variable = { .arg = &res, .fmt = 'b' }}}
        );
        THEN
            HCALL(
                    ((struct hotcall_functionconfig) { .function_id = hotcall_ecall_foo, .has_return = false })
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
