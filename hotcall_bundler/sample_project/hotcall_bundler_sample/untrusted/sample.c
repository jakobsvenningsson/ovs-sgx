#include <string.h>
#include <assert.h>

#include <unistd.h>
#include <pwd.h>
#include <libgen.h>
#include <stdlib.h>

#include <sgx_urts.h>
#include "sample.h"

#include "hotcall_bundler_sample_u.h"

#include "hotcall-untrusted.h"
#include "functions.h"
#include "examples.h"
#include "ovs-benchmark.h"
#include <gtest/gtest.h>
#include "test/test.h"

sgx_enclave_id_t global_eid = 0;

static struct shared_memory_ctx sm_ctx;

/* OCall functions */
void ocall_print(const char *str)
{
    printf("%s", str);
}

int initialize_enclave(void)
{
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
    ret = sgx_create_enclave(HOTCALL_BUNDLER_SAMPLE_FILENAME, SGX_DEBUG_FLAG, NULL, NULL, &global_eid, NULL);
    if (ret != SGX_SUCCESS) {
        printf("Failed to initialize enclave.\n");
        return -1;
    }
    return 0;
}

int cmpfunc (const void * a, const void * b) {
   return ( *(unsigned int*)a - *(unsigned int*)b );
}

#define ROUNDS 10000
#define WARM_UP ROUNDS/10
#define ITERATIONS 10
#define N_FOR_ITERS 10

unsigned int
 benchmark_bundle_if() {
    unsigned int rounds_1[ROUNDS];
    unsigned int rounds_2[ROUNDS];
    unsigned int rounds_3[ROUNDS];

    bool ret;
    for(int i = 0; i < (ROUNDS + WARM_UP); ++i) {
        /*long unsigned xs[32 * 1024];
        for(int j = 0; j < 32 * 1024; ++j) {
            xs[j] = rand();
        }
        BEGIN
        hotcall_bundle_begin(&sm_ctx, NULL);
        bool res1;
        HCALL(&sm_ctx, ecall_always_true, false, &res1, 0, NULL);

        int n_variables = 1;
        char fmt[] = "b";
        struct predicate_variable variables[n_variables] = {
            (struct predicate_variable) { &res1, VARIABLE_TYPE, 'b' }
        };
        struct if_args if_args = {
            .expected = 1,
            .then_branch_len = 1,
            .else_branch_len = 0,
            .fmt = fmt,
            .n_variables = n_variables,
            .variables = variables
        };
        IF(
            &sm_ctx,
            &if_args
        );
        THEN(
            HCALL(&sm_ctx, ecall_foo, false, NULL, 0, NULL)
        );
        ELSE(
            NULL
        );
        hotcall_bundle_end(&sm_ctx);
        CLOSE
        if(i >= WARM_UP) {
            rounds_1[i - WARM_UP] = GET_TIME
        }*/
    }

    qsort(rounds_1, ROUNDS, sizeof(unsigned int), cmpfunc);
    return rounds_1[ROUNDS / 2];
}

unsigned int
benchmark_vanilla_if() {
    unsigned int rounds_1[ROUNDS];
    bool ret;
    for(int i = 0; i < (ROUNDS + WARM_UP); ++i) {
        long unsigned xs[32 * 1024];
        for(int j = 0; j < 32 * 1024; ++j) {
            xs[j] = rand();
        }
        BEGIN
        HCALL(&sm_ctx, ecall_always_true, false, &ret, 0, NULL);
        if(ret) {
            HCALL(&sm_ctx, ecall_foo, false, NULL, 0, NULL)
        }
        CLOSE
        if(i < WARM_UP) {
            continue;
        }
        rounds_1[i - WARM_UP] = GET_TIME;
    }
    qsort(rounds_1, ROUNDS, sizeof(unsigned int), cmpfunc);
    return rounds_1[ROUNDS / 2];
}


unsigned int
benchmark_bundle_for_vanilla() {
    unsigned int rounds[ROUNDS];
    bool ret;
    for(int i = 0; i < (ROUNDS + WARM_UP); ++i) {
        //long unsigned xs[32 * 1024];
        //for(int j = 0; j < 32 * 1024; ++j) {
        //    xs[j] = rand();
       // }
        BEGIN
        void **args;
        for(int j = 0; j < N_FOR_ITERS; ++j) {
            args = sm_ctx.pfc.args[sm_ctx.pfc.idx];
            args[0] = (void *) &j;
            HCALL(&sm_ctx, ecall_plus_one, false, NULL, 1, args)
        }
        CLOSE
        if(i < WARM_UP) {
            continue;
        }
        rounds[i - WARM_UP] = GET_TIME;
    }
    qsort(rounds, ROUNDS, sizeof(unsigned int), cmpfunc);

    return rounds[ROUNDS / 2];
}


unsigned int
benchmark_bundle_for_naive() {
    unsigned int rounds[ROUNDS];
    bool ret;
    for(int i = 0; i < (ROUNDS + WARM_UP); ++i) {
        long unsigned xs[32 * 1024];
        for(int j = 0; j < 32 * 1024; ++j) {
            xs[j] = rand();
        }
        BEGIN
        hotcall_bundle_begin(&sm_ctx, NULL);
        void **args;
        int j;
        for(j = 0; j < N_FOR_ITERS; ++j) {
            args = sm_ctx.pfc.args[sm_ctx.pfc.idx];
            args[0] = (void *) &j;
            HCALL(&sm_ctx, ecall_plus_one, false, NULL, 1, args)
            //HCALL(&sm_ctx, ecall_plus_one, false, NULL, 1, args)
        }
        hotcall_bundle_end(&sm_ctx);
        CLOSE
        if(i < WARM_UP) {
            continue;
        }
        rounds[i - WARM_UP] = GET_TIME;
    }
    qsort(rounds, ROUNDS, sizeof(unsigned int), cmpfunc);
    return rounds[ROUNDS / 2];
}

unsigned int
benchmark_bundle_for_optimized() {
    unsigned int rounds[ROUNDS];
    bool ret;
    for(int i = 0; i < (ROUNDS + WARM_UP); ++i) {
        long unsigned dummy[32 * 1024];
        for(int j = 0; j < 32 * 1024; ++j) {
            dummy[j] = rand();
        }
        BEGIN
        hotcall_bundle_begin(&sm_ctx, NULL);
        int n_params = 1, n_rows = 1;
        int xs[N_FOR_ITERS] = { 0 };
        void *parameters[n_params] = { xs };
        char fmt[] = "d";
        /*FOR_EACH(
            &sm_ctx,
            ecall_plus_one,
            N_FOR_ITERS,
            n_params,
            parameters,
            fmt
        );*/
        hotcall_bundle_end(&sm_ctx);
        CLOSE
        if(i < WARM_UP) {
            continue;
        }
        rounds[i - WARM_UP] = GET_TIME;
    }
    qsort(rounds, ROUNDS, sizeof(unsigned int), cmpfunc);
    return rounds[ROUNDS / 2];
}

unsigned int
benchmark_bundle_for_v2() {
    unsigned int rounds[ROUNDS];
    bool ret;
    for(int i = 0; i < (ROUNDS + WARM_UP); ++i) {
        /*long unsigned dummy[32 * 1024];
        for(int j = 0; j < 32 * 1024; ++j) {
            dummy[j] = rand();
        }
        BEGIN
        hotcall_bundle_begin(&sm_ctx, NULL);
        unsigned int n_ecalls = 1;
        int xs[N_FOR_ITERS] = { 0 };
        void *args[1] = { xs };
        BEGIN_FOR(&sm_ctx, N_FOR_ITERS, n_ecalls);
        HCALL(&sm_ctx, ecall_plus_one, false, NULL, 1, args);
        //HCALL(&sm_ctx, ecall_plus_one, false, NULL, 1, args);
        END_FOR(&sm_ctx, n_ecalls);
        hotcall_bundle_end(&sm_ctx);
        CLOSE
        if(i < WARM_UP) {
            continue;
        }
        rounds[i - WARM_UP] = GET_TIME;*/
    }
    qsort(rounds, ROUNDS, sizeof(unsigned int), cmpfunc);
    return rounds[ROUNDS / 2];
}

unsigned int
benchmark_bundle_for_if() {
    unsigned int rounds[ROUNDS];
    bool ret;
    for(int i = 0; i < (ROUNDS + WARM_UP); ++i) {
        long unsigned dummy[32 * 1024];
        for(int j = 0; j < 32 * 1024; ++j) {
            dummy[j] = rand();
        }
        /*BEGIN
        hotcall_bundle_begin(&sm_ctx, NULL);
        unsigned int body_len = 3; // If operators counts as a ecall.
        int xs[N_FOR_ITERS] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        void *args[1] = { xs };
        BEGIN_FOR(&sm_ctx, N_FOR_ITERS, body_len);
        bool x;
        char fmt[] = "b";
        HCALL(&sm_ctx, ecall_always_false, false, &x, 0, NULL);
        IF(
            &sm_ctx,
            0,
            1,
            0,
            fmt,
            1,
            &x
        );
        THEN(HCALL(&sm_ctx, ecall_plus_one, false, NULL, 1, args));
        END_FOR(&sm_ctx, body_len);
        hotcall_bundle_end(&sm_ctx);
        CLOSE
        if(i < WARM_UP) {
            continue;
        }*/
        rounds[i - WARM_UP] = GET_TIME;
    }
    qsort(rounds, ROUNDS, sizeof(unsigned int), cmpfunc);
    return rounds[ROUNDS / 2];
}

unsigned int
benchmark_bundle_for_if_naive() {
    unsigned int rounds[ROUNDS];
    bool ret;
    for(int i = 0; i < (ROUNDS + WARM_UP); ++i) {
        /*long unsigned dummy[32 * 1024];
        for(int j = 0; j < 32 * 1024; ++j) {
            dummy[j] = rand();
        }
        BEGIN
        hotcall_bundle_begin(&sm_ctx, NULL);
        int xs[N_FOR_ITERS] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        for(int j = 0; j < N_FOR_ITERS; ++j) {
            bool x;
            char fmt[] = "b";
            HCALL(&sm_ctx, ecall_always_false, false, &x, 0, NULL);
            void *args[1] = { xs + j };
            IF(
                &sm_ctx,
                0,
                1,
                0,
                fmt,
                1,
                &x
            );
            THEN(HCALL(&sm_ctx, ecall_plus_one, false, NULL, 1, args));
        }
        hotcall_bundle_end(&sm_ctx);
        CLOSE
        if(i < WARM_UP) {
            continue;
        }
        rounds[i - WARM_UP] = GET_TIME;*/
    }
    qsort(rounds, ROUNDS, sizeof(unsigned int), cmpfunc);
    return rounds[ROUNDS / 2];
}


unsigned int
benchmark_bundle_for_if_optimized() {
    unsigned int rounds[ROUNDS];
    bool ret;
    for(int i = 0; i < (ROUNDS + WARM_UP); ++i) {
        long unsigned dummy[32 * 1024];
        for(int j = 0; j < 32 * 1024; ++j) {
            dummy[j] = rand();
        }
        BEGIN
        hotcall_bundle_begin(&sm_ctx, NULL);
        int n_params = 1, n_rows = 1;
        int xs[N_FOR_ITERS] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
        int xs_filtered[N_FOR_ITERS];

        unsigned int filtered_length;
        unsigned int n_iters = N_FOR_ITERS;
        char fmt[] = "d";
        void *parameters_filter[n_params] = { xs };
        void *parameters_for_each[n_params] = { xs_filtered };
        /*struct immutable_function_argument filter_args = {
            .n_params = n_params,
            .fmt = fmt,
            .params_in = parameters_filter,
            .params_in_length = N_FOR_ITERS,
            .params_out = parameters_for_each
        };
        FILTER(
          &sm_ctx,
          ecall_greater_than_two,
          &filter_args
      );*/
        /*FILTER(
            &sm_ctx,
            ecall_greater_than_two,
            n_params,
            parameters_filter,
            &n_iters,
            &filtered_length,
            parameters_for_each,
            fmt
        );*/
        /*struct immutable_function_argument for_each_args = {
            .n_params = filter_args.n_params,
            .fmt = filter_args.fmt,
            .params_in = filter_args.params_out,
            .params_in_length = filter_args.params_out_length
        };*/
        /*FOR_EACH(
          &sm_ctx,
          ecall_plus_one,
          &for_each_args
      );*/
    /*    FOR_EACH(
            &sm_ctx,
            ecall_plus_one,
            &filtered_length,
            n_params,
            parameters_for_each,
            fmt
        );*/
        hotcall_bundle_end(&sm_ctx);
        CLOSE
        if(i < WARM_UP) {
            continue;
        }
        rounds[i - WARM_UP] = GET_TIME;
    }
    qsort(rounds, ROUNDS, sizeof(unsigned int), cmpfunc);
    return rounds[ROUNDS / 2];
}

/*
TEST(foo,bar) {
    EXPECT_EQ( hotcall_test(), true );
}*/

/* Application entry */
int SGX_CDECL main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    /* Initialize the enclave */
    if(initialize_enclave() < 0){
        return -1;
    }



    //sleep(1);



    if(argc > 1 && !strcmp(argv[1], "-t")) {
        printf("Running test...\n");
        return hotcall_run_tests(&sm_ctx);
    }

    ecall_configure_hotcall(global_eid);
    hotcall_init(&sm_ctx, global_eid);

    hotcall_bundle_example_map(&sm_ctx);


    /* for(int n = 0; n < ITERATIONS; ++n) {
    printf("Running iteration %d.\n", n);
    unsigned int r1;
    r1 = benchmark_bundle_for_optimized();
    avg_median += r1;
    }
    printf("Median %u\n", avg_median/ITERATIONS);*/


  hotcall_destroy(&sm_ctx);

  //return RUN_ALL_TESTS();

  return 0;
}
