#include "gtest/gtest.h"
#include "test.h"
#include "hotcall-bundler-untrusted.h"
#include "functions.h"

#include "hotcall_if.h"

#define TEST_ERROR_CODE_1 1

TEST(error,1) {
    //Contarct: The if condition is false and hence the else branch should be executed. X should be incremented once and then the error is reached which should abort execution.

    hotcall_test_setup();
    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    BUNDLE_BEGIN();

    ASSERT_EQ(0, hotcall_bundle_get_error());

    int x = 0;
    bool res;
    HCALL(CONFIG( .function_id = hotcall_ecall_always_false, .has_return = true ), VAR(res, 'b'));
    IF((struct if_config) { .predicate_fmt = "b" }, VAR(res, 'b'));
    THEN
    ELSE
        HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(x, 'b'));
        ERROR(TEST_ERROR_CODE_1);
        HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(x, 'b'));
    END

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(x, 1);

    ASSERT_EQ(TEST_ERROR_CODE_1, hotcall_bundle_get_error());

}

TEST(error, 2) {
    // Contarct: The if condition is true and hence the "then" branch should be executed. X should be incremented once. Since no error statements is reached
    // hotcall_bundle_get_error should return NULL

    hotcall_test_setup();
    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    BUNDLE_BEGIN();

    // Check that error is 0 before we begin.
    ASSERT_EQ(0, hotcall_bundle_get_error());

    int x = 0;
    bool res;
    HCALL(CONFIG(.function_id = hotcall_ecall_always_false, .has_return = true), VAR(res, 'b'));
    IF((struct if_config) { .predicate_fmt = "!b" }, VAR(res, 'b'));
    THEN
        HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'b'));
    ELSE
        ERROR(TEST_ERROR_CODE_1);
    END

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(x, 1);
    ASSERT_EQ(0, hotcall_bundle_get_error());
}
