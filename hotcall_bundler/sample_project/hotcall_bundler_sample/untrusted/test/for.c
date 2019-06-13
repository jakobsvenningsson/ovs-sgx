#include "gtest/gtest.h"
#include "test.h"
#include "hotcall-untrusted.h"
#include "functions.h"

#include "hotcall_for.h"
/*
TEST(for,1) {
    //Contract: the hcalls inside the for loop should add 3 to each element in xs and 2 to each element in ys.
    hotcall_test_setup();
    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    hotcall_bundle_begin(sm_ctx);
    unsigned int n_iters = 10;
    int xs[n_iters] = { 0 };
    int ys[n_iters] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    BEGIN_FOR(sm_ctx, ((struct for_config) {
        .n_iters = &n_iters
    }));
    HCALL_1(
        sm_ctx,
        ((struct hotcall_function_config) {
            .f_id = hotcall_ecall_plus_plus,
            .has_return = false
        }),
        { .type = VARIABLE_TYPE_, .value = { .variable = { .arg = xs, .fmt = 'd', .iter = true }}},
        { .type = VARIABLE_TYPE_, .value = { .variable = { .arg = ys, .fmt = 'd', .iter = true }}}
    );
    HCALL_1(
        sm_ctx,
        ((struct hotcall_function_config) {
            .f_id = hotcall_ecall_plus_one,
            .has_return = false
        }),
        { .type = VARIABLE_TYPE_, .value = { .variable = { .arg = xs, .fmt = 'd', .iter = true }}}
    );
    HCALL_1(
        sm_ctx,
        ((struct hotcall_function_config) {
            .f_id = hotcall_ecall_plus_one,
            .has_return = false
        }),
        { .type = VARIABLE_TYPE_, .value = { .variable = { .arg = xs, .fmt = 'd', .iter = true }}}
    );
    END_FOR(sm_ctx);
    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();

    for(int i = 0; i < n_iters; ++i) {
        ASSERT_EQ(xs[i], 3);
        ASSERT_EQ(ys[i], i + 2);
    }
}

TEST(for,2) {
    //Contract: the "then" branch should not be executed for the first 3 iterations of the loop.
    hotcall_test_setup();
    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    hotcall_bundle_begin(sm_ctx);
    unsigned int n_params = 1, n_iters = 10;
    int xs[n_iters] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    bool ret;

    BEGIN_FOR(sm_ctx, { .n_iters = &n_iters });
    HCALL_1(sm_ctx, ((struct hotcall_function_config) { .f_id = hotcall_ecall_greater_than_two, .has_return = true }),
        (struct parameter) { .type = VARIABLE_TYPE_,   .value = { .variable = { .arg = xs, .fmt = 'd', .iter = true }}},
        (struct parameter) { .type = VARIABLE_TYPE_,   .value = { .variable = { .arg = &ret, .fmt = 'b', .iter = false }}},
    );
    IF(
        sm_ctx,
        ((struct if_config) {
            .then_branch_len = 1,
            .else_branch_len = 0,
            .predicate_fmt = "!b&b",
            .return_if_false = false
        }),
        (struct parameter) { .type = POINTER_TYPE_,    .value = { .pointer = { .arg = NULL }}},
        (struct parameter) { .type = VARIABLE_TYPE_,   .value = { .variable = { .arg = &ret, .fmt = 'b' }}}
    );
    THEN(
        HCALL_1(
            sm_ctx,
            ((struct hotcall_function_config) {
                .f_id = hotcall_ecall_plus_one,
                .has_return = false
            }),
            (struct parameter) { .type = VARIABLE_TYPE_,   .value = { .variable = { .arg = xs, .fmt = 'd', .iter = true }}}
        )
    );
    ELSE(NULL);

    END_FOR(sm_ctx);

    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();

    for(int i = 0; i < n_iters; ++i) {
        if(i > 2) {
            ASSERT_EQ(xs[i], i + 1);
        } else {
            ASSERT_EQ(xs[i], i);
        }
    }
}*/
