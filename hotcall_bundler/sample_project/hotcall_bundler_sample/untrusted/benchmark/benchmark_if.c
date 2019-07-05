#include "benchmark.h"
#include "hotcall-bundler-untrusted.h"
#include "functions.h"
#include "postfix_translator.h"


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

        BUNDLE_BEGIN();

        MAP(((struct map_config) {
                .function_id = hotcall_ecall_plus_one_ret,
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

        BUNDLE_BEGIN();

        BEGIN_FOR(((struct for_config) {
            .n_iters = &n_iters
        }));

        HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VECTOR(xs, 'd'));

        END_FOR();

        BUNDLE_END();

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
    int *xs_ptr[n_iters];

    int zs[n_iters];

    for(int i = 0; i < n_iters; ++i) {
        xs[i] = (i % 2) ? 3 : 1;
        xs_ptr[i] = &xs[i];
        //xs[i] = i > n_iters / 2 ? 3 : 1;
    }
    int y = 2;
    for(int i = 0; i < (n_rounds + warmup); ++i) {
        clear_cache();

        unsigned int out_length = 0;
        int ys[n_iters] = { 0 };


        BEGIN

        BUNDLE_BEGIN();


        /*FILTER(
            ((struct filter_config) { .predicate_fmt = "d>d" }),
            VECTOR(xs_ptr, 'd', &n_iters, .dereference = true), VAR(y, 'd'), VECTOR(ys, 'd', &out_length)
        );*/


        struct parameter function_parameter[] = {
            VECTOR(xs, 'd', &n_iters),
            VECTOR(zs, 'd', &n_iters)
        };

        FILTER(
            ((struct filter_config) { .predicate_fmt = "b" }),
            FUNC(.function_id = hotcall_ecall_greater_than_two, .params = function_parameter, .n_params = 2),
            VECTOR(ys, 'd', &out_length)
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
benchmark_for_each(struct shared_memory_ctx *sm_ctx, unsigned int n_rounds) {
    unsigned int warmup = n_rounds / 10;
    unsigned int rounds[n_rounds];


    unsigned int n_iters = 10000;
    int xs[n_iters] = { 0 };

    int *xs_ptr[n_iters];
    for(int i = 0; i < n_iters; ++i) {
        xs_ptr[i] = &xs[i];
    }

    for(int i = 0; i < (n_rounds + warmup); ++i) {
        clear_cache();

        BEGIN

        BUNDLE_BEGIN();

        FOR_EACH(((struct for_each_config) { .function_id = hotcall_ecall_plus_one, .n_iters = &n_iters}), VECTOR(xs_ptr, 'd', &n_iters, .dereference = true));

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
                ((struct hotcall_function_config) { .function_id = hotcall_ecall_always_true, .has_return = true }),
		        (struct parameter) { .type = VARIABLE_TYPE, .value = { .variable = { .arg = &res }}}
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

#include <unistd.h>

unsigned int
benchmark_if_optimized(struct shared_memory_ctx *sm_ctx, unsigned int n_rounds) {
    unsigned int warmup = n_rounds / 10;
    unsigned int rounds[n_rounds];
    for(int i = 0; i < (n_rounds + warmup); ++i) {
        clear_cache();
        unsigned n_iters = 100, out_length;
        int xs[n_iters] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        int ys[n_iters] = { 0 };

        int x = 0, y = 0;
        struct parameter param[] = { VAR(x, 'y') };

        struct parameter function_parameter[] = {
            VECTOR(xs, 'd', &n_iters),
            VECTOR(ys, 'd', &n_iters)
        };

        BEGIN

        /*

        for(int i = 0; i < n_iters; ++i) {
            HCALL(CONFIG(.function_id = hotcall_ecall_always_true, .has_return = true), VAR(y, 'd'));
            if(y) {
                HCALL(CONFIG(.function_id = hotcall_ecall_always_true, .has_return = true), VAR(y, 'd'));
            }
        }

        */


        BUNDLE_BEGIN();


            FILTER(((struct filter_config) {
                .predicate_fmt = "b"
            }),
            FUNC(.function_id = hotcall_ecall_greater_than_two, .params = function_parameter, .n_params = 2),
            VECTOR(ys, 'd', &out_length));

            FOR_EACH(
                ((struct for_each_config) { .function_id = hotcall_ecall_plus_one, .n_iters = &n_iters }),
                VECTOR(ys, 'd')
            );

        BUNDLE_END();



        /*BUNDLE_BEGIN();

            BEGIN_FOR(((struct for_config) {
                .n_iters = &n_iters
            }));

            //    ASSERT(3, FUNC(hotcall_ecall_always_true, .params = param, .n_params = 1));
                HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VECTOR(ys, 'd'));

            END_FOR();

        BUNDLE_END();*/



        CLOSE
        SHOWTIME5

        unsigned int t = GET_TIME
        printf("t: %u\n", t);


        if(i >= warmup) {
            rounds[i - warmup] = GET_TIME
        }
    }
    qsort(rounds, n_rounds, sizeof(unsigned int), cmpfunc);
    return rounds[n_rounds / 2];
}
