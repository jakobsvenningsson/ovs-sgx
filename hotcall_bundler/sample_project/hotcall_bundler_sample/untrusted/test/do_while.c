#include "gtest/gtest.h"
#include "test.h"
#include "hotcall-untrusted.h"
#include "functions.h"

#include "hotcall_do_while.h"


TEST(do_while,1) {
    //Contract: the body of the while loop should execute 3 times and hence x should be 3 after termination.

    hotcall_test_setup();

    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    hotcall_bundle_begin(sm_ctx);

    int x = 0;

    struct parameter function_parameter[] = {
        { .type = VARIABLE_TYPE_, .value = { .variable = { .arg = &x, .fmt = 'd' }}}
    };


    DO_WHILE(
        sm_ctx,
        ((struct do_while_config) {
            .function_id = hotcall_ecall_plus_one,
            .condition_fmt = "!b"
        }),
        CONDITION_PARAMS(
            (struct parameter) { .type = FUNCTION_TYPE_,
                                 .value = { .function = { .function_id = hotcall_ecall_greater_than_two, .params = function_parameter, .n_params = 1}}}
        ),
        FUNCTION_PARAMS(
            (struct parameter) { .type = VARIABLE_TYPE_,
                                 .value = { .variable = { .arg = &x, .fmt = 'd', .iter = false }}}
        )
    );

    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();

    ASSERT_EQ(x, 3);
}

TEST(do_while,2) {
    //Contract: the body of the while loop should execute 3 times and hence x should be 3 after termination.

    hotcall_test_setup();

    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    hotcall_bundle_begin(sm_ctx);

    int x = 0, y = 5;

    struct parameter function_parameter[] = {
        { .type = VARIABLE_TYPE_, .value = { .variable = { .arg = &x, .fmt = 'd' }}}
    };


    DO_WHILE(
        sm_ctx,
        ((struct do_while_config) {
            .function_id = hotcall_ecall_plus_one,
            .condition_fmt = "d<d&!b"
        }),
        CONDITION_PARAMS(
            (struct parameter) { .type = VARIABLE_TYPE_,
                                 .value = { .variable = { .arg = &x, .fmt = 'd', .iter = false }}},
            (struct parameter) { .type = VARIABLE_TYPE_,
                                 .value = { .variable = { .arg = &y, .fmt = 'd', .iter = false }}},
            (struct parameter) { .type = FUNCTION_TYPE_,
                                 .value = { .function = { .function_id = hotcall_ecall_greater_than_two, .params = function_parameter, .n_params = 1 }}},
        ),
        FUNCTION_PARAMS(
            (struct parameter) { .type = VARIABLE_TYPE_,
                                 .value = { .variable = { .arg = &x, .fmt = 'd', .iter = false }}}
        )
    );

    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();

    ASSERT_EQ(x, 3);
}
