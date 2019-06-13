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

    MAP(
        sm_ctx,
        ((struct map_config) {
            .function_id = hotcall_ecall_plus_one_ret,
            .n_params = n_params
        }),
        (struct parameter) { .type = VARIABLE_TYPE_,
                            .value = { .variable = { .arg = xs, .fmt = 'd', .iter = true }},
                            .len = &n_iters },
        (struct parameter) { .type = VARIABLE_TYPE_,
                            .value = { .variable = { .arg = ys, .fmt = 'd', .iter = true }}}
    );

    hotcall_bundle_end(sm_ctx);

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
    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    hotcall_bundle_begin(sm_ctx);

    unsigned int n_params = 2, n_iters = 10;
    int xs[n_iters] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    int ys[n_iters] = { 0 };
    int z = 1;

    MAP(
        sm_ctx,
        ((struct map_config) {
            .function_id = hotcall_ecall_plus,
            .n_params = n_params
        }),
        (struct parameter) { .type = VARIABLE_TYPE_,
                            .value = { .variable = { .arg = xs, .fmt = 'd', .iter = true }},
                            .len = &n_iters },
        (struct parameter) { .type = VARIABLE_TYPE_,
                            .value = { .variable = { .arg = &z, .fmt = 'd', .iter = false }}},
        (struct parameter) { .type = VARIABLE_TYPE_,
                            .value = { .variable = { .arg = ys, .fmt = 'd', .iter = true }}}
    );

    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();

    ASSERT_EQ(n_iters, 10);
    for(int i = 0; i < n_iters; ++i) {
        ASSERT_EQ(ys[i], i + z);
        ASSERT_EQ(xs[i], i);
    }
}
