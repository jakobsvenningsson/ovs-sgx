#include "gtest/gtest.h"
#include "test.h"
#include "hotcall-untrusted.h"
#include "functions.h"


TEST(map,1) {

    hotcall_test_setup();
    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    hotcall_bundle_begin(sm_ctx, NULL);

    unsigned int n_params = 1, n_iters = 10;
    int xs[n_iters] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    int ys[n_iters] = { 0 };

    struct function_parameter params_in[n_params] = {
        (struct function_parameter) { .arg = xs, .fmt = 'd', .iter = true }
    };

    struct map_args map_args = {
        .params_in = (struct function_parameters_in) {
            .params = params_in, .n_params = n_params, .iters = n_iters
        },
        .params_out = (struct function_map_out) {
            .params = ys, .fmt = 'd'
        }
    };

    MAP(
        sm_ctx,
        ecall_plus_one_ret,
        &map_args
    );

    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();

    for(int i = 0; i < n_iters; ++i) {
        ASSERT_EQ(ys[i], i + 1);
    }
}


TEST(map,2) {

    hotcall_test_setup();
    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    hotcall_bundle_begin(sm_ctx, NULL);

    unsigned int n_params = 2, n_iters = 10;
    int xs[n_iters] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    int ys[n_iters] = { 0 };
    int x = 3;

    struct function_parameter params_in[n_params] = {
        (struct function_parameter) { .arg = xs, .fmt = 'd', .iter = true },
        (struct function_parameter) { .arg = &x, .fmt = 'd', .iter = false }
    };

    struct map_args map_args = {
        .params_in = (struct function_parameters_in) {
            .params = params_in, .n_params = n_params, .iters = n_iters
        },
        .params_out = (struct function_map_out) {
            .params = ys, .fmt = 'd'
        }
    };

    MAP(
        sm_ctx,
        ecall_plus,
        &map_args
    );

    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();

    for(int i = 0; i < n_iters; ++i) {
        ASSERT_EQ(ys[i], i + x);
    }
}
