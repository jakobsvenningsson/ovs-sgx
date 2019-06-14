#include "gtest/gtest.h"
#include "test.h"
#include "hotcall-untrusted.h"
#include "hotcall.h"
#include "functions.h"

#include "hotcall_filter.h"
#include "hotcall_map.h"


TEST(chaining,1) {
    hotcall_test_setup();

    BUNDLE_BEGIN();
    unsigned int n_params = 2, n_iters = 10, out_length1, out_length2;
    int xs[n_iters] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    int ys[n_iters] = { 0 };
    int ws[n_iters] = { 0 };
    int us[n_iters] = { 0 };
    int ts[n_iters] = { 0 };
    int ks[n_iters] = { 0 };

    int y = 6, z = 2, w = 3, v = 14;

    struct parameter function_parameters[] = { VECTOR(xs, 'd', &n_iters), VAR(y, 'd') };

    CHAIN_BEGIN();

    FILTER(
        ((struct filter_config) { .condition_fmt = "b" }),
        FUNC(.function_id = hotcall_ecall_greater_than_y, .params = function_parameters, .n_params = 2),
        VECTOR(ys, 'd', &out_length1)
    );
    MAP(
        ((struct map_config) { .function_id = hotcall_ecall_plus }),
        VECTOR(),
        VAR(z, 'd'), VECTOR(ws, 'd', &out_length1)
    );
    MAP(
        ((struct map_config) { .function_id = hotcall_ecall_plus }),
        VECTOR(),
        VAR(w, 'd'), VECTOR(us, 'd', &out_length1)
    );
    MAP(
        ((struct map_config) { .function_id = hotcall_ecall_plus_one_ret }),
        VECTOR(),
        VECTOR(ts, 'd', &out_length1)
    );
    FILTER(
        ((struct filter_config) { .condition_fmt = "d>d" }),
        VECTOR(),
        VAR(v, 'd'),
        VECTOR(ks, 'd', &out_length2)
    );

    CHAIN_CLOSE();

    BUNDLE_END();

    hotcall_test_teardown();

    // check that orignal list is unmodified.
    for(int i = 0; i < n_iters; ++i) {
        ASSERT_EQ(xs[i], i);
    }
    ASSERT_EQ(out_length1, 3);
    for(int i = 0; i < out_length1; ++i) {
        ASSERT_EQ(ws[i], i + 7 + z);
    }
    for(int i = 0; i < out_length1; ++i) {
        ASSERT_EQ(us[i], i + 7 + z + w);
    }
    for(int i = 0; i < out_length1; ++i) {
        ASSERT_EQ(ts[i], i + 7 + z + w + 1);
    }
    ASSERT_EQ(out_length2, 1);
    for(int i = 0; i < out_length2; ++i) {
        ASSERT_EQ(ks[i], i + 9 + z + w + 1);
    }
}


TEST(chaining, 2) {
    hotcall_test_setup();

    BUNDLE_BEGIN();

    unsigned int n_iters = 10;
    int xs[n_iters] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    int ys[n_iters] = { 0 };

    int y = 2, res = 0;


    CHAIN_BEGIN();

    MAP(
        ((struct map_config) { .function_id = hotcall_ecall_plus }),
        VECTOR(xs, 'd', &n_iters),
        VAR(y, 'd'),
        VECTOR(ys, 'd', &n_iters)
    );
    REDUCE(
        ((struct reduce_config) {
            .function_id = 255,
            .op = '+',
        }),
        VECTOR(),
        VAR(res, 'd')
    );

    CHAIN_CLOSE();

    BUNDLE_END();

    hotcall_test_teardown();


    for(int i = 0; i < n_iters; ++i) {
        ASSERT_EQ(xs[i], i);
    }

    for(int i = 0; i < n_iters; ++i) {
        ASSERT_EQ(ys[i], i + 2);
    }

    for(int i = 0; i < n_iters; ++i) {
        ASSERT_EQ(res, (1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9) + 2*10);
    }

}


/*
    //Contract: Filter should filter out all elements with a value less than 3,

    hotcall_test_setup();
    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    BUNDLE_BEGIN();
    unsigned int n_params = 1, n_iters = 10, out_length;
    int xs[n_iters] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    int ys[n_iters] = { true, true, true };
    bool zs[n_iters];
    bool ws[n_iters];
    bool us[n_iters];

    // Filter arguments
    struct function_parameter function_parameter_filter_in[n_params] = {
        (struct function_parameter) { .arg = xs, .fmt = 'd', .iter = true }
    };
    struct function_parameters_in params_in = {
        .params = function_parameter_filter_in, .n_params = n_params, .iters = &n_iters
    };

    // Filter output
    struct function_parameter function_parameter_filter_out[n_params] = {
        (struct function_parameter) { .arg = ys, .fmt = 'd', .iter = true }
    };
    struct function_parameters_in params_out = {
        .params = function_parameter_filter_out, .n_params = n_params, .iters = &out_length
    };

    // Filter predicate
    struct hotcall_function fc = {
        .id = hotcall_ecall_greater_than_two
    };
    unsigned int n_variables = 1;
    char fmt[] = "!b";
    struct predicate_variable variables[n_variables] = {
        (struct predicate_variable) { &fc, FUNCTION_TYPE, 'b' }
    };

    hotcall_bundle_chain_begin(sm_ctx);
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

    // Map output
    struct function_parameter function_paramete_out_map[n_params] = {
        (struct function_parameter) { .arg = zs, .fmt = 'b'  }
    };
    struct function_parameters_in params_out_map = {
        .params = function_paramete_out_map, .n_params = n_params, .iters = &out_length
    };
    MAP(
        sm_ctx,
        ecall_greater_than_two,
        ((struct map_args) {
            .params_in = NULL,
            .params_out = &params_out_map
        })
    );

    // Map output
    struct function_parameter function_paramete_out_map_final[n_params] = {
        (struct function_parameter) { .arg = ws, .fmt = 'b'  }
    };
    struct function_parameters_in params_out_map_final = {
        .params = function_paramete_out_map_final, .n_params = n_params, .iters = &out_length
    };
    MAP(
        sm_ctx,
        ecall_revert,
        ((struct map_args) {
            .params_in = NULL,
            .params_out = &params_out_map_final
        })
    );

    // Filter arguments
    struct function_parameter function_parameter_filter_in_final[n_params] = {
        (struct function_parameter) { .arg = ws, .fmt = 'b', .iter = true }
    };
    struct function_parameters_in params_in_filter_final = {
        .params = function_parameter_filter_in_final, .n_params = n_params, .iters = &out_length
    };

    // Filter output
    struct function_parameter function_parameter_filter_out_final[n_params] = {
        (struct function_parameter) { .arg = us, .fmt = 'b', .iter = true }
    };
    struct function_parameters_in params_out_filter_final = {
        .params = function_parameter_filter_out_final, .n_params = n_params, .iters = &out_length
    };

    // Filter predicate
    unsigned int n_variables_final = 1;
    char fmt_final[] = "b";

    struct predicate_variable variables_final[n_variables_final] = {
        (struct predicate_variable) { ws, VARIABLE_TYPE, 'b', .iter = true }
    };

    FILTER(
        sm_ctx,
        ((struct filter_args) {
            .params_in = &params_in_filter_final,
            .params_out = &params_out_filter_final,
            .predicate = (struct predicate)  {
                .fmt = fmt_final,
                .n_variables = n_variables_final,
                .variables = variables_final
            }
        })
    );




    hotcall_bundle_chain_close(sm_ctx);

    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();

    ASSERT_EQ(out_length, 3);

    // check that orignal list is unmodified.
    for(int i = 0; i < n_iters; ++i) {
        ASSERT_EQ(xs[i], i);
    }
    for(int i = 0; i < out_length; ++i) {
        ASSERT_EQ(ys[i], i);
    }
    for(int i = 0; i < out_length; ++i) {
        ASSERT_EQ(zs[i], false);
    }
    for(int i = 0; i < out_length; ++i) {
        ASSERT_EQ(ws[i], true);
    }
    for(int i = 0; i < out_length; ++i) {
        ASSERT_EQ(us[i], true);
    }
}
*/
