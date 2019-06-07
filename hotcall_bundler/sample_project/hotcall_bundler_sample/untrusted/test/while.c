#include "gtest/gtest.h"
#include "test.h"
#include "hotcall-untrusted.h"
#include "functions.h"

#include "hotcall_while.h"


TEST(while,1) {
    // Contract: the loop should execute twice and increment x with 2 in each iteration. Therefore x should be 4 when the loop terminates.
    hotcall_test_setup();
    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    hotcall_bundle_begin(sm_ctx);
    unsigned int n_ecalls = 2, n_params = 1, n_variables = 3;
    char fmt[] = "!(b&b|b)";
    int x = 0;
    bool b = false;
    void *args[n_params] = { &x };
    struct hotcall_function fc = {
        .id = hotcall_ecall_greater_than_two,
        .args = (struct hotcall_function_arg_list) {
            .n_args = 1,
            .args = { &x }
        }
    };
    struct predicate_variable variables[n_variables] = {
        (struct predicate_variable) { &fc, FUNCTION_TYPE, 'b' },
        (struct predicate_variable) { &fc, FUNCTION_TYPE, 'b' },
        (struct predicate_variable) { &b, VARIABLE_TYPE, 'b' }
    };
    struct predicate predicate = {
        .fmt = fmt,
        .n_variables = n_variables,
        .variables = variables
    };
    struct while_args while_args = {
        .predicate = predicate,
        .n_rows = n_ecalls
    };
    BEGIN_WHILE(
        sm_ctx,
        &while_args
    );
    HCALL(sm_ctx, ecall_plus_one, false, NULL, n_params, args);
    HCALL(sm_ctx, ecall_plus_one, false, NULL, n_params, args);
    END_WHILE(sm_ctx, &while_args);
    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();

    ASSERT_EQ(x, 4);
}

TEST(while, 2) {
    // Contract: the loop should execute 10 times and hence x should be 10 after the loop has terminated.
    hotcall_test_setup();
    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    int x = 0;
    int y = 10;
    hotcall_bundle_begin(sm_ctx);
    unsigned int n_ecalls = 1, n_params = 1, n_variables = 2;
    char fmt[] = "d<d";
    void *args[n_params] = { &x };
    struct predicate_variable variables[n_variables] = {
        (struct predicate_variable) { &x, VARIABLE_TYPE, 'b' },
        (struct predicate_variable) { &y, VARIABLE_TYPE, 'b' }
    };
    struct predicate predicate = {
        .fmt = fmt,
        .n_variables = n_variables,
        .variables = variables
    };
    struct while_args while_args = {
        .predicate = predicate,
        .n_rows = n_ecalls
    };
    BEGIN_WHILE(
        sm_ctx,
        &while_args
    );
    HCALL(sm_ctx, ecall_plus_one, false, NULL, n_params, args);
    END_WHILE(sm_ctx, &while_args);
    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();

    ASSERT_EQ(x, 10);
}
