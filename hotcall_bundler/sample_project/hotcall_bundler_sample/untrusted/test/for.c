#include "gtest/gtest.h"
#include "test.h"
#include "hotcall-untrusted.h"
#include "functions.h"

#include "hotcall_for.h"

TEST(for,1) {
    //Contract: the hcalls inside the for loop should add 3 to each element in xs and 2 to each element in ys.
    hotcall_test_setup();
    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    hotcall_bundle_begin(sm_ctx);
    unsigned int n_ecalls = 3, n_params = 2, n_iters = 10;
    int xs[n_iters] = { 0 };
    int ys[n_iters] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    void *args[n_params] = { xs, ys };

    struct for_args for_args = {
        .n_iters = n_iters,
        .n_rows = n_ecalls
    };
    BEGIN_FOR(sm_ctx, &for_args);
    HCALL(sm_ctx, ecall_plus_plus, false, NULL, 2, args);
    HCALL(sm_ctx, ecall_plus_one, false, NULL, 1, args);
    HCALL(sm_ctx, ecall_plus_one, false, NULL, 1, args);
    END_FOR(sm_ctx, &for_args);
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
    unsigned int n_ecalls = 3, n_params = 1, n_iters = 10, n_variables = 2;
    int xs[n_iters] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    void *args[n_params] = { xs };

    struct for_args for_args = {
        .n_iters = n_iters,
        .n_rows = n_ecalls
    };

    char fmt[] = "b&b";

    struct hotcall_function fc = {
        .id = hotcall_ecall_greater_than_two,
        .args = (struct hotcall_function_arg_list) {
            .n_args = 1,
            .args = { xs }
        }
    };

    struct predicate_variable variables[n_variables] = {
        (struct predicate_variable) { NULL, VARIABLE_TYPE, 'b' },
        (struct predicate_variable) { &fc, FUNCTION_TYPE, 'b' }
    };

    struct if_args if_args = {
        .then_branch_len = 1,
        .else_branch_len = 0,
        .predicate = (struct predicate) {
            .fmt = fmt,
            .n_variables = n_variables,
            .variables = variables
        },
        .return_if_false = false
    };

    BEGIN_FOR(sm_ctx, &for_args);
    bool ret;
    HCALL(sm_ctx, ecall_greater_than_two, false, &ret, 1, args);
    if_args.predicate.variables[0].val = &ret;
    IF(
        sm_ctx,
        &if_args
    );
    THEN(HCALL(sm_ctx, ecall_plus_one, false, NULL, 1, args));
    ELSE(NULL);

    END_FOR(sm_ctx, &for_args);
    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();

    for(int i = 0; i < n_iters; ++i) {
        if(i > 2) {
            ASSERT_EQ(xs[i], i + 1);
        } else {
            ASSERT_EQ(xs[i], i);
        }
    }
}
