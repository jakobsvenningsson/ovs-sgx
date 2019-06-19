#include "gtest/gtest.h"
#include "test.h"
#include "hotcall-bundler-untrusted.h"
#include "functions.h"

#include "hotcall_map.h"

TEST(map,1) {
    //Contract: Map shoud add 1 to each element in the output list. The input list shall be unmodified.
    hotcall_test_setup();

    BUNDLE_BEGIN();

    unsigned int n_iters = 10;
    int xs[n_iters] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    int ys[n_iters] = { 0 };

    struct parameter vec1[] = { VAR(xs, 'd'), VECTOR_v2(&vec1[0], &n_iters) }, p1 = vec1[1];
    struct parameter vec2[] = { VAR(ys, 'd'), VECTOR_v2(&vec2[0], &n_iters) }, p2 = vec2[1];

    MAP(
        ((struct map_config) { .function_id = hotcall_ecall_plus_one_ret }),
        p1, p2
    );

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(n_iters, 10);
    for(int i = 0; i < n_iters; ++i) {
        ASSERT_EQ(ys[i], i + 1);
        ASSERT_EQ(xs[i], i);
    }
}

TEST(map,2) {
    //Contract: Map should add z to each element in the output list. The input list shall be unmodified.
    hotcall_test_setup();

    BUNDLE_BEGIN();

    unsigned int n_iters = 10;
    int xs[n_iters] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    int ys[n_iters] = { 0 };
    struct parameter vec1[] = { VAR(xs, 'd'), VECTOR_v2(&vec1[0], &n_iters) }, p1 = vec1[1];
    struct parameter vec2[] = { VAR(ys, 'd'), VECTOR_v2(&vec2[0], &n_iters) }, p2 = vec2[1];
    int z = 1;

    MAP(
        ((struct map_config) { .function_id = hotcall_ecall_plus }),
        p1,
        VAR(z, 'd'),
        p2
    );

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(n_iters, 10);
    for(int i = 0; i < n_iters; ++i) {
        ASSERT_EQ(ys[i], i + z);
        ASSERT_EQ(xs[i], i);
    }
}

TEST(map,3) {
    //Contract: Map should add z to each element in the output list. The input list shall be unmodified.
    hotcall_test_setup();

    BUNDLE_BEGIN();

    unsigned int n_iters = 10;

    int xs[n_iters] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    int ys[n_iters] = { 0 };
    int z = 1;

    struct parameter vec1[] = { VAR(xs, 'd'), VECTOR_v2(&vec1[0], &n_iters) }, p1 = vec1[1];
    struct parameter vec2[] = { VAR(ys, 'd'), VECTOR_v2(&vec2[0], &n_iters) }, p2 = vec2[1];

    MAP(
        ((struct map_config) { .function_id = hotcall_ecall_plus }),
        VAR(z, 'd'), p1, p2
    );

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(n_iters, 10);
    for(int i = 0; i < n_iters; ++i) {
        ASSERT_EQ(ys[i], i + z);
        ASSERT_EQ(xs[i], i);
    }
}
