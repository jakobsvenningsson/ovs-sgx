#include "gtest/gtest.h"
#include "test.h"
#include "hotcall-bundler-untrusted.h"
#include "functions.h"

#include "hotcall_filter.h"

TEST(filter,1) {
    //Contract: Filter should filter out all elements with a value less than 3,

    hotcall_test_setup();

    BUNDLE_BEGIN();

    unsigned int n_iters = 10, out_length;
    int xs[n_iters] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    int ys[n_iters] = { 0 };

    struct parameter function_parameter[] = { VECTOR(xs, 'd', &n_iters) };

    FILTER(
        ((struct filter_config) { .condition_fmt = "b" }),
        FUNC(.function_id = hotcall_ecall_greater_than_two, .params = function_parameter, .n_params = 1),
        VECTOR(ys, 'd', &out_length)
    );

    BUNDLE_END();

    hotcall_test_teardown();

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

    unsigned int n_iters = 10, out_length;
    int xs[n_iters] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    int ys[n_iters] = { 0 };
    int y = 6;

    BUNDLE_BEGIN();

    FILTER(
        ((struct filter_config) { .condition_fmt = "d>d" }),
        VECTOR(xs, 'd', &n_iters),
        VAR(y, 'd'),
        VECTOR(ys, 'd', &out_length)
    );

    BUNDLE_END();

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

    BUNDLE_BEGIN();

    unsigned int n_iters = 10, out_length;
    int xs[n_iters] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    int ys[n_iters] = { 0 };
    int y = 6;

    struct parameter function_parameters[] = { VECTOR(xs, 'd', &n_iters), VAR(y, 'd') };

    FILTER(
        ((struct filter_config) { .condition_fmt = "b" }),
        FUNC(.function_id = hotcall_ecall_greater_than_y, .params = function_parameters, .n_params = 2),
        VECTOR(ys, 'd', &out_length)
    );

    BUNDLE_END();

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
