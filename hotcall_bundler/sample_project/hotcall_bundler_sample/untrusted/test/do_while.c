#include "gtest/gtest.h"
#include "test.h"
#include "hotcall-bundler-untrusted.h"
#include "functions.h"

#include "hotcall_do_while.h"


TEST(do_while,1) {
    //Contract: the body of the while loop should execute 3 times and hence x should be 3 after termination.

    hotcall_test_setup();

    int x = 0; bool b;
    struct parameter function_parameter[] = { VAR(x, 'd'), VAR(b, 'b') };

    BUNDLE_BEGIN();

    DO_WHILE(
        ((struct do_while_config) {
            .function_id = hotcall_ecall_plus_one,
            .predicate_fmt = "!b"
        }),
        CONDITION_PARAMS(FUNC(.function_id = hotcall_ecall_greater_than_two, .params = function_parameter, .n_params = 2)),
        FUNCTION_PARAMS(VAR(x, 'd'))
    );

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(x, 3);
}

TEST(do_while,2) {
    //Contract: the body of the while loop should execute 3 times and hence x should be 3 after termination.

    hotcall_test_setup();

    int x = 0, y = 5;
    bool b;
    struct parameter function_parameter[] = { VAR(x, 'd'), VAR(b, 'b') };

    BUNDLE_BEGIN();

    DO_WHILE(
        ((struct do_while_config) {
            .function_id = hotcall_ecall_plus_one,
            .predicate_fmt = "d<d&!b"
        }),
        CONDITION_PARAMS(VAR(x, 'd'),
                         VAR(y, 'd'),
                         FUNC(.function_id = hotcall_ecall_greater_than_two, .params = function_parameter, .n_params = 2)),
        FUNCTION_PARAMS(VAR(x, 'd'))
    );

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(x, 3);
}
