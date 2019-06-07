#include "gtest/gtest.h"
#include "test.h"
#include "hotcall-untrusted.h"
#include "functions.h"

#include "hotcall_if.h"

#define TEST_ERROR_CODE_1 1

TEST(error,1) {
    //Contarct: The if condition is false and hence the else branch should be executed. X should be incremented once and then the error is reached which should abort execution.

    hotcall_test_setup();
    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    hotcall_bundle_begin(sm_ctx);

    bool res;
    HCALL(sm_ctx, ecall_always_false, false, &res, 0, NULL);


    int n_variables = 1;
    char fmt[] = "b";
    struct predicate_variable variables[n_variables] = {
        (struct predicate_variable) { &res, VARIABLE_TYPE, 'b' },
    };
    struct if_args if_args = {
        .then_branch_len = 0,
        .else_branch_len = 3,
        .predicate = (struct predicate) {
            .fmt = fmt,
            .n_variables = n_variables,
            .variables = variables
        },
        .return_if_false = false
    };

    int x = 0;
    void *args[1] = { &x };

    IF(
        sm_ctx,
        &if_args
    );
    THEN(
        NULL
    );
    ELSE(
        HCALL(sm_ctx, ecall_plus_one, false, NULL, 1, args);
        ERROR(sm_ctx, TEST_ERROR_CODE_1);
        HCALL(sm_ctx, ecall_plus_one, false, NULL, 1, args);
    );

    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();

    ASSERT_EQ(x, 1);
    ASSERT_EQ(TEST_ERROR_CODE_1, hotcall_bundle_get_error(sm_ctx));

}
