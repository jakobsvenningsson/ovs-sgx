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
