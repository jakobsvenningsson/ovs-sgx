#include "gtest/gtest.h"
#include "test.h"
#include "hotcall-untrusted.h"
#include "functions.h"

#include "hotcall_for_each.h"



TEST(for_each,1) {
    //Contract: for each should apply the hcall on each element in the list
    hotcall_test_setup();
    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    hotcall_bundle_begin(sm_ctx);

    unsigned int n_params = 1, n_iters = 10;
    int xs[n_iters] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    FOR_EACH(
        sm_ctx,
        ((struct for_each_config) {
            .function_id = hotcall_ecall_plus_one
        }),
        (struct parameter) { .type = VARIABLE_TYPE_,
                             .value = { .variable = { .arg = xs, .fmt = 'd', .iter = true }}, .len = &n_iters }
    );

    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();
    for(int i = 0; i < n_iters; ++i) {
        ASSERT_EQ(xs[i], i + 1);
    }
}

TEST(for_each,2) {
    //Contract: for each should apply the hcall on each element in the list. All elements of xs should be 1 and all elements of ys 2.
    hotcall_test_setup();
    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    hotcall_bundle_begin(sm_ctx);

    unsigned int n_params = 1, n_iters = 10;
    int xs[n_iters] = { 0 };
    int ys[n_iters] = { 0 };

    FOR_EACH(
        sm_ctx,
        ((struct for_each_config) {
            .function_id = hotcall_ecall_plus_plus
        }),
        (struct parameter) { .type = VARIABLE_TYPE_,
                             .value = { .variable = { .arg = xs, .fmt = 'd', .iter = true }}, .len = &n_iters },
        (struct parameter) { .type = VARIABLE_TYPE_,
                             .value = { .variable = { .arg = ys, .fmt = 'd', .iter = true }}, .len = &n_iters }
    );

    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();
    for(int i = 0; i < n_iters; ++i) {
        ASSERT_EQ(xs[i], 1);
        ASSERT_EQ(ys[i], 2);
    }
}

TEST(for_each,3) {
    //Contract: for each should apply the hcall on each element in the list. All elements of xs should be 5 in the end.
    hotcall_test_setup();
    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    hotcall_bundle_begin(sm_ctx);

    unsigned int n_params = 1, n_iters = 10;
    int xs[n_iters] = { 0 };
    int y = 5;

    FOR_EACH(
        sm_ctx,
        ((struct for_each_config) {
            .function_id = hotcall_ecall_plus_y
        }),
        (struct parameter) { .type = VARIABLE_TYPE_,
                             .value = { .variable = { .arg = xs, .fmt = 'd', .iter = true }}, .len = &n_iters },
        (struct parameter) { .type = VARIABLE_TYPE_,
                             .value = { .variable = { .arg = &y, .fmt = 'd', .iter = false }} }
    );

    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();
    for(int i = 0; i < n_iters; ++i) {
        ASSERT_EQ(xs[i], 5);
    }
}
