#include "gtest/gtest.h"
#include "test.h"
#include "hotcall-bundler-untrusted.h"
#include "functions.h"

#include "hotcall_while.h"

TEST(while,1) {
    // Contract: the loop should execute twice and increment x with 2 in each iteration. Therefore x should be 4 when the loop terminates.
    hotcall_test_setup();

    int x = 0;
    bool b = false, b1;
    struct parameter function_parameters[] = { VAR(x, 'd'), VAR(b1, 'b') };

    BUNDLE_BEGIN();

    BEGIN_WHILE(
        ((struct while_config) { .predicate_fmt = "!(b&b|b)" }),
        FUNC(.function_id = hotcall_ecall_greater_than_two, .params = function_parameters, .n_params = 2),
        FUNC(.function_id = hotcall_ecall_greater_than_two, .params = function_parameters, .n_params = 2),
        VAR(b, 'b')
    );

        HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));
        HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));

    END_WHILE();

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(x, 4);
}



TEST(while, 2) {
    // Contract: the loop should execute 10 times and hence x should be 10 after the loop has terminated.
    hotcall_test_setup();

    int x = 0, y = 10;

    BUNDLE_BEGIN();

    BEGIN_WHILE(
        ((struct while_config) { .predicate_fmt = "d<d" }),
        VAR(x, 'd'), VAR(y, 'd')
    );

        HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));

    END_WHILE();

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(x, 10);
}

TEST(while, 3) {
    // Contract: the loop should execute 10 times and hence x should be 10 after the loop has terminated.
    hotcall_test_setup();

    int x = 0, y = 10, z = 0, counter = 0;

    BUNDLE_BEGIN();

    BEGIN_WHILE(
        ((struct while_config) { .predicate_fmt = "d<d" }),
        VAR(x, 'd'), VAR(y, 'd')
    );

        HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));

        BEGIN_WHILE(
            ((struct while_config) { .predicate_fmt = "d<d" }),
            VAR(z, 'd'), VAR(y, 'd')
        );

            HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(z, 'd'));
            HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(counter, 'd'));

        END_WHILE();

        HCALL(CONFIG( .function_id = hotcall_ecall_zero ), VAR(z, 'd'));

    END_WHILE();

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(counter, 100);
}

TEST(while, 4) {
    // Contract:
    hotcall_test_setup();

    int xs[10] = { 0 }, y = 10, z = 0;

    BUNDLE_BEGIN();

    BEGIN_WHILE(
        ((struct while_config) { .predicate_fmt = "d<d", .iter_vectors = true }),
        VAR(z, 'd'), VAR(y, 'd')
    );
        HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(z, 'd'));
        HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VECTOR(xs, 'd'));

    END_WHILE();

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(z, 10);
    for(int i = 0; i < 10; ++i) {
        ASSERT_EQ(xs[i], 1);
    }
}
