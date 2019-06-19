#include "gtest/gtest.h"
#include "test.h"
#include "hotcall-bundler-untrusted.h"
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

    struct parameter vec1[] = { VAR(xs, 'd'), VECTOR_v2(&vec1[0], &n_iters) }, p1 = vec1[1];
    struct parameter vec2[] = { VAR(ys, 'd'), VECTOR_v2(&vec2[0], &out_length1) }, p2 = vec2[1];
    struct parameter vec3[] = { VAR(ws, 'd'), VECTOR_v2(&vec3[0], &out_length1) }, p3 = vec3[1];
    struct parameter vec4[] = { VAR(us, 'd'), VECTOR_v2(&vec4[0], &out_length1) }, p4 = vec4[1];
    struct parameter vec5[] = { VAR(ts, 'd'), VECTOR_v2(&vec5[0], &out_length1) }, p5 = vec5[1];
    struct parameter vec6[] = { VAR(ks, 'd'), VECTOR_v2(&vec6[0], &out_length2) }, p6 = vec6[1];

    int y = 6, z = 2, w = 3, v = 14;

    struct parameter function_parameters[] = { p1, VAR(y, 'd') };

    CHAIN_BEGIN();

    FILTER(
        ((struct filter_config) { .condition_fmt = "b" }),
        FUNC(.function_id = hotcall_ecall_greater_than_y, .params = function_parameters, .n_params = 2),
        p2
    );
    MAP(
        ((struct map_config) { .function_id = hotcall_ecall_plus }),
        VECTOR_v2(NULL),
        VAR(z, 'd'), p3
    );
    MAP(
        ((struct map_config) { .function_id = hotcall_ecall_plus }),
        VECTOR_v2(NULL),
        VAR(w, 'd'), p4
    );
    MAP(
        ((struct map_config) { .function_id = hotcall_ecall_plus_one_ret }),
        VECTOR_v2(NULL),
        p5
    );
    FILTER(
        ((struct filter_config) { .condition_fmt = "d>d" }),
        VECTOR_v2(NULL),
        VAR(v, 'd'),
        p6
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

    struct parameter vec1[] = { VAR(xs, 'd'), VECTOR_v2(&vec1[0], &n_iters) }, p1 = vec1[1];
    struct parameter vec2[] = { VAR(ys, 'd'), VECTOR_v2(&vec2[0], &n_iters) }, p2 = vec2[1];

    CHAIN_BEGIN();

    MAP(
        ((struct map_config) { .function_id = hotcall_ecall_plus }),
        p1,
        VAR(y, 'd'),
        p2
    );
    REDUCE(
        ((struct reduce_config) {
            .function_id = 255,
            .op = '+',
        }),
        VECTOR_v2(NULL),
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
