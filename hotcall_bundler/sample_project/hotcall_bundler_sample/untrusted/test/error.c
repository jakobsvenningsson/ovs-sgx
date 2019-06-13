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
    int x = 0;
    bool res;
    HCALL(CONFIG({ .function_id = hotcall_ecall_always_false, .has_return = true }), VAR(&res, 'b'));
    IF(
        ((struct if_config) {
            .then_branch_len = 0,
            .else_branch_len = 3,
            .predicate_fmt = "b",
            .return_if_false = false
        }),
        VAR(&res, 'b')
    );
    THEN
        NULL;
    ELSE
        HCALL(CONFIG({ .function_id = hotcall_ecall_plus_one, .has_return = false }), VAR(&x, 'b'));
        ERROR(sm_ctx, TEST_ERROR_CODE_1);
        HCALL(CONFIG({ .function_id = hotcall_ecall_plus_one, .has_return = false }), VAR(&x, 'b'));

    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();

    ASSERT_EQ(x, 1);
    ASSERT_EQ(TEST_ERROR_CODE_1, hotcall_bundle_get_error(sm_ctx));

}

TEST(error, 2) {
    // Contarct: The if condition is true and hence the "then" branch should be executed. X should be incremented once. Since no error statements is reached
    // hotcall_bundle_get_error should return NULL

    hotcall_test_setup();
    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    hotcall_bundle_begin(sm_ctx);
    int x = 0;
    bool res;
    HCALL(CONFIG({ .function_id = hotcall_ecall_always_false, .has_return = true }), VAR(&res, 'b'));
    IF(
        ((struct if_config) {
            .then_branch_len = 1,
            .else_branch_len = 1,
            .predicate_fmt = "!b",
            .return_if_false = false
        }),
        VAR(&res, 'b')
    );
    THEN
        HCALL(CONFIG({ .function_id = hotcall_ecall_plus_one, .has_return = false }), VAR(&x, 'b'));
    ELSE
        ERROR(sm_ctx, TEST_ERROR_CODE_1);


    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();

    ASSERT_EQ(x, 1);
    ASSERT_EQ(NULL, hotcall_bundle_get_error(sm_ctx));
}
