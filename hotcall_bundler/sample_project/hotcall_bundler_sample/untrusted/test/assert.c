#include "gtest/gtest.h"
#include "test.h"
#include "hotcall-bundler-untrusted.h"
#include "functions.h"
#include "hotcall_assert.h"

TEST(assert, 1) {
    hotcall_test_setup();

    int x = 1, y = 0, error_code = 3;

    BUNDLE_BEGIN();

    HCALL(CONFIG(.function_id = hotcall_ecall_always_false, .has_return = true), VAR(x, 'd'));
    ASSERT(true, error_code, VAR(x, 'd'));
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(y, 'd'));

    BUNDLE_END();

    ASSERT_EQ(x, 0);
    ASSERT_EQ(y, 0);
    ASSERT_EQ(3, hotcall_bundle_get_error());

    BUNDLE_BEGIN();

    HCALL(CONFIG(.function_id = hotcall_ecall_always_true, .has_return = true), VAR(x, 'd'));
    ASSERT(true, error_code, VAR(x, 'd'));
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(y, 'd'));

    BUNDLE_END();

    ASSERT_EQ(x, 1);
    ASSERT_EQ(y, 1);
    ASSERT_EQ(0, hotcall_bundle_get_error());

    hotcall_test_teardown();
}

TEST(assert, 2) {
    hotcall_test_setup();

    int x = 1, y = 0, error_code = 3;
    struct parameter param[] = { VAR(x, 'y') };

    BUNDLE_BEGIN();

    ASSERT(true, error_code, FUNC(hotcall_ecall_always_false, .params = param, .n_params = 1));
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(y, 'd'));

    BUNDLE_END();

    ASSERT_EQ(x, 0);
    ASSERT_EQ(y, 0);
    ASSERT_EQ(3, hotcall_bundle_get_error());

    BUNDLE_BEGIN();

    ASSERT(true, error_code, FUNC(hotcall_ecall_always_true, .params = param, .n_params = 1));
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(y, 'd'));

    BUNDLE_END();

    ASSERT_EQ(x, 1);
    ASSERT_EQ(y, 1);
    ASSERT_EQ(0, hotcall_bundle_get_error());

    hotcall_test_teardown();

}
