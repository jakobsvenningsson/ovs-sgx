#include "gtest/gtest.h"
#include "test.h"
#include "hotcall-untrusted.h"
#include "functions.h"

#include "hotcall_map.h"

TEST(map,1) {
    //Contract: Map shoud add 1 to each element in the output list. The input list shall be unmodified.
    hotcall_test_setup();
    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    hotcall_bundle_begin(sm_ctx);

    unsigned int n_params = 1, n_iters = 10;
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
        .params = function_params_out, .n_params = n_params, .iters = &n_iters
    };

    MAP(
        sm_ctx,
        ecall_plus_one_ret,
        ((struct map_args) {
            .params_in = &params_in,
            .params_out = &params_out
        })
    );

    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();


    for(int i = 0; i < n_iters; ++i) {
        ASSERT_EQ(ys[i], i + 1);
        ASSERT_EQ(xs[i], i);
    }
}


TEST(map,2) {
    //Contract: Map shoud add x to each element in the output list. The input list shall be unmodified.

    hotcall_test_setup();
    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    hotcall_bundle_begin(sm_ctx);

    unsigned int n_params = 2, n_iters = 10;
    int xs[n_iters] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    int ys[n_iters] = { 0 };
    int x = 3;

    struct function_parameter function_params_in[n_params] = {
        (struct function_parameter) { .arg = xs, .fmt = 'd', .iter = true },
        (struct function_parameter) { .arg = &x, .fmt = 'd', .iter = false }
    };
    struct function_parameters_in params_in = {
        .params = function_params_in, .n_params = n_params, .iters = &n_iters
    };
    struct function_parameter function_params_out[1] = {
        (struct function_parameter) { .arg = ys, .fmt = 'd' }
    };
    struct function_parameters_in params_out = {
        .params = function_params_out, .n_params = 1, .iters = &n_iters
    };

    MAP(
        sm_ctx,
        ecall_plus,
        ((struct map_args) {
            .params_in = &params_in,
            .params_out = &params_out
        })
    );

    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();

    for(int i = 0; i < n_iters; ++i) {
        ASSERT_EQ(ys[i], i + x);
        ASSERT_EQ(xs[i], i);
    }
}
