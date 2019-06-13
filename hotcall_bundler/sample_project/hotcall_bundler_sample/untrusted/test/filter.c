#include "gtest/gtest.h"
#include "test.h"
#include "hotcall-untrusted.h"
#include "functions.h"

#include "hotcall_filter.h"

TEST(filter,1) {
    //Contract: Filter should filter out all elements with a value less than 3,

    hotcall_test_setup();
    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    hotcall_bundle_begin(sm_ctx);
    unsigned int n_params = 1, n_iters = 10, out_length;
    int xs[n_iters] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    int ys[n_iters] = { 0 };

    struct parameter function_parameter[] = {
        { .type = VARIABLE_TYPE_, .value = { .variable = { .arg = xs, .fmt = 'd' , .iter = true }}}
    };

    FILTER(
        sm_ctx,
        ((struct filter_config) {
            .condition_fmt = "b"
        }),
        (struct parameter) { .type = FUNCTION_TYPE_,
                            .value = { .function = { .function_id = hotcall_ecall_greater_than_two, .params = function_parameter, .n_params = 1}},
                            .len = &n_iters },
        (struct parameter) { .type = VARIABLE_TYPE_,
                            .value = { .variable = { .arg = ys, .fmt = 'd', .iter = false }},
                            .len = &out_length }
    );

    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();


    for(int i = 0; i < 10; ++i) {
        printf("%d ", ys[i]);
    }
    printf("\n");

    // check that orignal list is unmodified.
    for(int i = 0; i < n_iters; ++i) {
        ASSERT_EQ(xs[i], i);
    }

    ASSERT_EQ(out_length, 7);
    for(int i = 0; i < out_length; ++i) {
        ASSERT_EQ(ys[i], i + 3);
    }
}


TEST(filter,2) {
    //Contract: Filter should filter out all elements with a value less than 7,

    hotcall_test_setup();
    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    hotcall_bundle_begin(sm_ctx);
    unsigned int n_params = 2, n_iters = 10, out_length;
    int xs[n_iters] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    int ys[n_iters] = { 0 };
    int y = 6;
    FILTER(
        sm_ctx,
        ((struct filter_config) {
            .condition_fmt = "d>d"
        }),
        (struct parameter) { .type = VARIABLE_TYPE_,
                            .value = { .variable = { .arg = xs, .fmt = 'd', .iter = true }},
                            .len = &n_iters },
        (struct parameter) { .type = VARIABLE_TYPE_,
                            .value = { .variable = { .arg = &y, .fmt = 'd', .iter = false }}},
        (struct parameter) { .type = VARIABLE_TYPE_,
                            .value = { .variable = { .arg = ys, .fmt = 'd', .iter = true }},
                            .len = &out_length }
    );

    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();

    // check that orignal list is unmodified.
    for(int i = 0; i < n_iters; ++i) {
        ASSERT_EQ(xs[i], i);
    }
    ASSERT_EQ(out_length, 3);
    for(int i = 0; i < out_length; ++i) {
        ASSERT_EQ(ys[i], i + 7);
    }
}


TEST(filter,3) {
    //Contract: Filter should filter out all elements with a value less than 7,

    hotcall_test_setup();
    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    hotcall_bundle_begin(sm_ctx);
    unsigned int n_params = 2, n_iters = 10, out_length;
    int xs[n_iters] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    int ys[n_iters] = { 0 };
    int y = 6;

    struct parameter function_parameters[] = {
        { .type = VARIABLE_TYPE_, .value = { .variable = { .arg = xs, .fmt = 'd' , .iter = true }}},
        { .type = VARIABLE_TYPE_, .value = { .variable = { .arg = &y, .fmt = 'd' , .iter = false }}}
    };

    FILTER(
        sm_ctx,
        ((struct filter_config) {
            .condition_fmt = "b"
        }),
        (struct parameter) { .type = FUNCTION_TYPE_,
                            .value = { .function = { .function_id = hotcall_ecall_greater_than_y, .params = function_parameters, .n_params = 2 }},
                            .len = &n_iters },
        (struct parameter) { .type = VARIABLE_TYPE_,
                            .value = { .variable = { .arg = ys, .fmt = 'd', .iter = true }},
                            .len = &out_length }
    );

    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();

    // check that orignal list is unmodified.
    for(int i = 0; i < n_iters; ++i) {
        ASSERT_EQ(xs[i], i);
    }
    ASSERT_EQ(out_length, 3);
    for(int i = 0; i < out_length; ++i) {
        ASSERT_EQ(ys[i], i + 7);
    }
}
