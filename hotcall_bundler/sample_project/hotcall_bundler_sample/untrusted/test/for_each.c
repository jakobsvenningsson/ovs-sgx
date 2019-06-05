#include "gtest/gtest.h"
#include "test.h"
#include "hotcall-untrusted.h"
#include "functions.h"

#include "hotcall_for_each.h"

TEST(for_each,1) {
    //Contract: for each should apply the hcall on each element in the list
    hotcall_test_setup();
    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    hotcall_bundle_begin(sm_ctx, NULL);

    unsigned int n_params = 1, n_iters = 10;
    int xs[n_iters] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    struct function_parameter params_in[n_params] = {
        (struct function_parameter) { .arg = xs, .fmt = 'd', .iter = true }
    };
    struct for_each_args for_each_args = {
        .params_in = (struct function_parameters_in) {
            .params = params_in, .n_params = 1, .iters = n_iters
        },
    };
    FOR_EACH(
        sm_ctx,
        ecall_plus_one,
        &for_each_args
    );
    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();
    for(int i = 0; i < n_iters; ++i) {
        ASSERT_EQ(xs[i], i + 1);
    }
}
