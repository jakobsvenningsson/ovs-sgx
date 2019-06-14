#include "gtest/gtest.h"
#include "test.h"
#include "hotcall-untrusted.h"
#include "hotcall.h"
#include "functions.h"

TEST(hotcall, 1) {
    /* Contract: When a hotcall is executed outside a 'bundle' then it shall block and execute immediately. */
    hotcall_test_setup();

    int x = 0;
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));

    hotcall_test_teardown();

    ASSERT_EQ(x, 1);
}

TEST(hotcall, 2) {
    /* Contract: When a hotcall is executed inside a 'bundle' then the hotcall shall be buffered and the method call shall return immediately.
       When the bundle is closed, execution is blocked and all buffered hotcalls will be executed. */

    hotcall_test_setup();

    BUNDLE_BEGIN();

    int x = 0;
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));

    // x shall still be 0 since the previous hotcall has only been schedueled and buffered but not executed.
    ASSERT_EQ(x, 0);

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(x, 1);
}

TEST(hotcall, 3) {
    /* Contract: When a function is marked to return a value, the last parameter given as input to HCALL (in this case y)
       will be used to store the return value. */

    hotcall_test_setup();

    BUNDLE_BEGIN();

    int x = 0, y = 0;
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one_ret, .has_return = true ), VAR(x, 'd'), VAR(y, 'd'));

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(x, 0);
    ASSERT_EQ(y, 1);
}

TEST(hotcall, 4) {
    /* Contract:  */

    hotcall_test_setup();

    BUNDLE_BEGIN();

    int x = 0, y = 0, z = 0;
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one_ret, .has_return = true ), VAR(x, 'd'), VAR(y, 'd'));
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one_ret, .has_return = true ), VAR(y, 'd'), VAR(z, 'd'));

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(x, 0);
    ASSERT_EQ(y, 1);
    ASSERT_EQ(z, 2);

}
