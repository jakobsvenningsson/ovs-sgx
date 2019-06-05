#include "gtest/gtest.h"
#include "test.h"
#include "hotcall-untrusted.h"
#include "functions.h"


TEST(filter,1) {
    //Contract: Filter should filter out all elements with a value less than 3,

    hotcall_test_setup();
    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    hotcall_bundle_begin(sm_ctx, NULL);
    unsigned int n_params = 1, n_iters = 10, out_length;
    int xs[n_iters] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    int ys[n_iters] = { 0 };

    struct function_parameter params_in[n_params] = {
        (struct function_parameter) { .arg = xs, .fmt = 'd', .iter = true }
    };

    struct function_parameter params_out[n_params] = {
        (struct function_parameter) { .arg = ys, .fmt = 'd', .iter = true }
    };

    struct function_call fc = {
        .id = hotcall_ecall_greater_than_two
    };

    unsigned int n_variables = 1;
    char fmt[] = "b";
    struct predicate_variable variables[n_variables] = {
        (struct predicate_variable) { &fc, FUNCTION_TYPE, 'b' }
    };

    struct filter_args filter_args = {
        .params_in = (struct function_parameters_in) {
            .params = params_in, .n_params = 1, .iters = n_iters
        },
        .params_out = (struct function_filter_out) {
            .params = params_out, .len = &out_length
        },
        .predicate = (struct predicate)  {
            .expected = 1,
            .fmt = fmt,
            .n_variables = n_variables,
            .variables = variables
        }
    };

    FILTER(
        sm_ctx,
        ecall_greater_than_two,
        &filter_args
    );

    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();

    // check that orignal list is unmodified.
    for(int i = 0; i < n_iters; ++i) {
        ASSERT_EQ(xs[i], i);
    }

    for(int i = 0; i < out_length; ++i) {
        ASSERT_EQ(ys[i], i + 3);
    }
}


TEST(filter,2) {
    //Contract: Filter should filter out all elements with a value less than 7,


    hotcall_test_setup();
    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    hotcall_bundle_begin(sm_ctx, NULL);
    char fmt[] = "b&b";
    char fmt_input[] = "dd";
    unsigned int n_params = 2, n_iters = 10, out_length;
    int xs[n_iters] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    int ys[n_iters] = { 0 };

    int y = 6;

    struct function_parameter params_in[n_params] = {
        (struct function_parameter) { .arg = xs, .fmt = 'd', .iter = true },
        (struct function_parameter) { .arg = &y, .fmt = 'd', .iter = false }
    };

    struct function_parameter params_out[n_params] = {
        (struct function_parameter) { .arg = ys, .fmt = 'd', .iter = true },
        (struct function_parameter) { .arg = &y, .fmt = 'd', .iter = false }
    };

    struct function_call fc = {
        .id = hotcall_ecall_greater_than_y
    };

    bool z = true;

    unsigned int n_variables = 2;
    struct predicate_variable variables[n_variables] = {
        (struct predicate_variable) { &z, VARIABLE_TYPE, 'b' },
        (struct predicate_variable) { &fc, FUNCTION_TYPE, 'b' }
    };

    struct filter_args filter_args = {
        .params_in = (struct function_parameters_in) {
            .params = params_in, .n_params = 2, .iters = n_iters,
        },
        .params_out = (struct function_filter_out) {
            .params = params_out, .len = &out_length
        },
        .predicate = (struct predicate)  {
            .expected = 1,
            .fmt = fmt,
            .n_variables = n_variables,
            .variables = variables
        }

    };

    FILTER(
        sm_ctx,
        ecall_greater_than_y,
        &filter_args
    );

  hotcall_bundle_end(sm_ctx);

  hotcall_test_teardown();

  // check that orignal list is unmodified.
  for(int i = 0; i < n_iters; ++i) {
      ASSERT_EQ(xs[i], i);
  }

  for(int i = 0; i < out_length; ++i) {
      ASSERT_EQ(ys[i], i + 7);
  }
}
