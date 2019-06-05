#include "gtest/gtest.h"
#include "test.h"
#include "hotcall-untrusted.h"
#include "functions.h"

#include "hotcall_if.h"

TEST(if,1) {
    // Contract: a true condition should execute the "then" branch
    hotcall_test_setup();
    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    int x = 0;
    hotcall_bundle_begin(sm_ctx, NULL);
    bool res1, res2, res3;
    bool *ptr = NULL;

    HCALL(sm_ctx, ecall_always_false, false, &res1, 0, NULL);
    HCALL(sm_ctx, ecall_always_true, false, &res2, 0, NULL);
    HCALL(sm_ctx, ecall_always_true, false, &res3, 0, NULL);

    int n_variables = 4;
    char fmt[] = "!b&(b|b)&b";
    struct predicate_variable variables[n_variables] = {
        (struct predicate_variable) { ptr, POINTER_TYPE, 'b' },
        (struct predicate_variable) { &res1, VARIABLE_TYPE, 'b' },
        (struct predicate_variable) { &res2, VARIABLE_TYPE, 'b' },
        (struct predicate_variable) { &res3, VARIABLE_TYPE, 'b' },
    };

    struct predicate predicate = {
        .expected = 1,
        .fmt = fmt,
        .n_variables = n_variables,
        .variables = variables
    };

    struct if_args if_args = {
        .then_branch_len = 1,
        .else_branch_len = 2,
        .predicate = predicate,
        .return_if_false = false
    };

    void *args[1] = { &x };

    IF(
        sm_ctx,
        &if_args
    );
    THEN(
        HCALL(sm_ctx, ecall_plus_one, false, NULL, 1, args)
    );
    ELSE(
        HCALL(sm_ctx, ecall_plus_one, false, NULL, 1, args);
        HCALL(sm_ctx, ecall_plus_one, false, NULL, 1, args);
    );

    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();

    ASSERT_EQ(x, 1);
}

TEST(if,2) {
    // Contract: a false condition should execute the "else" branch
    hotcall_test_setup();

    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    int x = 0;
    hotcall_bundle_begin(sm_ctx, NULL);

    bool res;
    HCALL(sm_ctx, ecall_always_false, false, &res, 0, NULL);

    struct hotcall_function fc = {
        .id = hotcall_ecall_always_true
    };

    int n_variables = 2;
    char fmt[] = "b|!b";
    struct predicate_variable variables[n_variables] = {
        (struct predicate_variable) { &res, VARIABLE_TYPE, 'b' },
        (struct predicate_variable) { &fc, FUNCTION_TYPE, 'b' },
    };

    struct predicate predicate = {
        .expected = 1,
        .fmt = fmt,
        .n_variables = n_variables,
        .variables = variables
    };

    struct if_args if_args = {
        .then_branch_len = 1,
        .else_branch_len = 2,
        .predicate = predicate,
        .return_if_false = false
    };

    void *args[1] = { &x };

    IF(
        sm_ctx,
        &if_args
    );
    THEN(
        HCALL(sm_ctx, ecall_plus_one, false, NULL, 1, args)
    );
    ELSE(
        HCALL(sm_ctx, ecall_plus_one, false, NULL, 1, args);
        HCALL(sm_ctx, ecall_plus_one, false, NULL, 1, args);
    );

    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();

    ASSERT_EQ(x, 2);
}


TEST(if,3) {
    // Contract: a false condition and "return_if_false = true" shall stop execution after else branch.
    hotcall_test_setup();

    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    int x = 0;
    hotcall_bundle_begin(sm_ctx, NULL);

    bool res;
    HCALL(sm_ctx, ecall_always_false, false, &res, 0, NULL);

    struct hotcall_function fc = {
        .id = hotcall_ecall_always_true
    };

    int n_variables = 2;
    char fmt[] = "b|!b";
    struct predicate_variable variables[n_variables] = {
        (struct predicate_variable) { &res, VARIABLE_TYPE, 'b' },
        (struct predicate_variable) { &fc, FUNCTION_TYPE, 'b' },
    };

    struct predicate predicate = {
        .expected = 1,
        .fmt = fmt,
        .n_variables = n_variables,
        .variables = variables
    };

    struct if_args if_args = {
        .then_branch_len = 1,
        .else_branch_len = 2,
        .predicate = predicate,
        .return_if_false = true
    };

    void *args[1] = { &x };

    IF(
        sm_ctx,
        &if_args
    );
    THEN(
        HCALL(sm_ctx, ecall_plus_one, false, NULL, 1, args)
    );
    ELSE(
        HCALL(sm_ctx, ecall_plus_one, false, NULL, 1, args);
        HCALL(sm_ctx, ecall_plus_one, false, NULL, 1, args);
    );

    HCALL(sm_ctx, ecall_plus_one, false, NULL, 1, args);

    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();

    ASSERT_EQ(x, 2);
}
