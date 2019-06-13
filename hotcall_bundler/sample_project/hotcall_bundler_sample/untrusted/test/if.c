#include "gtest/gtest.h"
#include "test.h"
#include "hotcall-untrusted.h"
#include "functions.h"

#include "hotcall_if.h"
#include "hotcall.h"

TEST(if,1) {
    // Contract: a true condition should execute the "then" branch
    hotcall_test_setup();

    int x = 0;
    bool res1, res2, res3;
    bool *ptr = NULL;

    BUNDLE_BEGIN();

    HCALL(CONFIG( .function_id = hotcall_ecall_always_false, .has_return = true ), VAR(&res1, 'b'));
    HCALL(CONFIG( .function_id = hotcall_ecall_always_true, .has_return = true ), VAR(&res2, 'b'));
    HCALL(CONFIG( .function_id = hotcall_ecall_always_true, .has_return = true ), VAR(&res3, 'b'));
    IF(
        ((struct if_config) { .predicate_fmt = "!b&(b|b)&b" }),
        PTR(ptr), VAR(&res1, 'b'), VAR(&res2, 'b'), VAR(&res3, 'b')
    );
    THEN
        HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(&x, 'd'));
    ELSE
        HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(&x, 'd'));
        HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(&x, 'd'));
    END

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(x, 1);
}

TEST(if,2) {
    // Contract: a false condition should execute the "else" branch
    hotcall_test_setup();

    int x = 0;
    bool res;

    BUNDLE_BEGIN();

    HCALL(CONFIG( .function_id = hotcall_ecall_always_false, .has_return = true ), VAR(&res, 'b'));
    IF(
        ((struct if_config) { .predicate_fmt = "b|!b" }),
        VAR(&res, 'b'),
        FUNC(hotcall_ecall_always_true, NULL)
    );
    THEN
        HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(&x, 'd'));
    ELSE
        HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(&x, 'd'));
        HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(&x, 'd'));
    END

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(x, 2);
}

TEST(if,3) {
    // Contract: a false condition and "return_if_false = true" shall stop execution after else branch.
    hotcall_test_setup();

    int x = 0;
    bool res;
    int a = 6, b = 3;
    struct parameter function_parameters[] = { VAR(&a, 'd'), VAR(&b, 'd') };

    BUNDLE_BEGIN();

    HCALL(CONFIG( .function_id = hotcall_ecall_always_false, .has_return = true ), VAR(&res, 'b'));
    IF(
        ((struct if_config) { .predicate_fmt = "b|!b" }),
        VAR( &res, 'b' ), FUNC( .function_id = hotcall_ecall_greater_than_y, .params = function_parameters, .n_params = 2 )
    );
    THEN
        HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(&x, 'd'));
        HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(&x, 'd'));
        HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(&x, 'd'));
        HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(&x, 'd'));
    ELSE
        HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(&x, 'd'));
        HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(&x, 'd'));
        RETURN;
    END

    HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(&x, 'd'));

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(0, hotcall_bundle_get_error());

    ASSERT_EQ(x, 2);
}

TEST(if,4) {
    hotcall_test_setup();

    int x = 0;
    bool res;

    BUNDLE_BEGIN();

    HCALL(CONFIG( .function_id = hotcall_ecall_always_true ), VAR(&res, 'b'));
    IF(
        ((struct if_config) { .predicate_fmt = "b" }),
        VAR(&res, 'b')
    );
    THEN
        HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(&x, 'd'));
    END

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(x, 1);
}



TEST(if,5) {
    hotcall_test_setup();

    int x = 0;

    BUNDLE_BEGIN();

    IF(
        ((struct if_config) { .predicate_fmt = "b" }),
        FUNC(.function_id = hotcall_ecall_always_true, .params = NULL)
    );
    THEN
        HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(&x, 'd'));
    END

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(x, 1);
}

TEST(if,6) {
    hotcall_test_setup();

    int x = 0;

    BUNDLE_BEGIN();

    IF(
        ((struct if_config) { .predicate_fmt = "!b" }),
        FUNC(.function_id = hotcall_ecall_always_false, .params =  NULL)
    );
    THEN
        HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(&x, 'd'));
    END

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(x, 1);
}
