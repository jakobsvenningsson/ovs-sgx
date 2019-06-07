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

    struct function_parameter function_params_in[n_params] = {
        (struct function_parameter) { .arg = xs, .fmt = 'd', .iter = true }
    };
    struct function_parameters_in params_in = {
        .params = function_params_in, .n_params = 1, .iters = &n_iters
    };

    struct function_parameter function_params_out[n_params] = {
        (struct function_parameter) { .arg = ys, .fmt = 'd', .iter = true }
    };
    struct function_parameters_in params_out = {
        .params = function_params_out, .n_params = 1, .iters = &out_length
    };

    struct hotcall_function fc = {
        .id = hotcall_ecall_greater_than_two
    };

    unsigned int n_variables = 1;
    char fmt[] = "b";
    struct predicate_variable variables[n_variables] = {
        (struct predicate_variable) { &fc, FUNCTION_TYPE, 'b', .iter = false }
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

    hotcall_bundle_begin(sm_ctx);
    char fmt[] = "b&b";
    unsigned int n_params = 2, n_iters = 10, out_length;
    int xs[n_iters] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    int ys[n_iters] = { 0 };

    int y = 6;

    struct function_parameter function_params_in[n_params] = {
        (struct function_parameter) { .arg = xs, .fmt = 'd', .iter = true },
        (struct function_parameter) { .arg = &y, .fmt = 'd', .iter = false }
    };
    struct function_parameters_in params_in = {
        .params = function_params_in, .n_params = 2, .iters = &n_iters
    };

    struct function_parameter function_params_out[n_params] = {
        (struct function_parameter) { .arg = ys, .fmt = 'd', .iter = true },
        (struct function_parameter) { .arg = &y, .fmt = 'd', .iter = false }
    };
    struct function_parameters_in params_out = {
        .params = function_params_out, .n_params = 2, .iters = &out_length
    };

    struct hotcall_function fc = {
        .id = hotcall_ecall_greater_than_y
    };
    bool z = true;
    unsigned int n_variables = 2;
    struct predicate_variable variables[n_variables] = {
        (struct predicate_variable) { &z, VARIABLE_TYPE, 'b', .iter = false },
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

    unsigned int n_params = 1, n_iters = 10, out_length;
    int xs[n_iters] = { 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };
    int ys[n_iters] = { 0 };

    struct function_parameter function_params_in[n_params] = {
        (struct function_parameter) { .arg = xs, .fmt = 'd', .iter = true }
    };
    struct function_parameters_in params_in = {
        .params = function_params_in, .n_params = 1, .iters = &n_iters
    };


    struct function_parameter function_params_out[n_params] = {
        (struct function_parameter) { .arg = ys, .fmt = 'd', .iter = true }
    };
    struct function_parameters_in params_out = {
        .params = function_params_out, .n_params = 1, .iters = &out_length
    };

    char fmt[] = "d>d";
    int y = 10;
    unsigned int n_variables = 2;
    struct predicate_variable variables[n_variables] = {
        (struct predicate_variable) { xs, VARIABLE_TYPE, 'b', .iter = true },
        (struct predicate_variable) { &y,  VARIABLE_TYPE, 'b', .iter = false }
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

    hotcall_test_teardown();

    // check that orignal list is unmodified.
    for(int i = 0; i < n_iters; ++i) {
        ASSERT_EQ(xs[i], i + 5);
    }
    ASSERT_EQ(out_length, 4);
    for(int i = 0; i < out_length; ++i) {
        ASSERT_TRUE(ys[i] > 10);
    }

}
