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
    int x = 0;
    bool b = false;

    struct parameter function_parameters[] = {
        (struct parameter) { .type = VARIABLE_TYPE_,   .value = { .variable = { .arg = &x, .fmt = 'd', .iter = false }}}
    };

    BEGIN_WHILE(
        ((struct while_config) {
            .predicate_fmt = "!(b&b|b)"
        }),
        (struct parameter) { .type = FUNCTION_TYPE_,   .value = { .function = { .function_id = hotcall_ecall_greater_than_two, .params = function_parameters, .n_params = 1 }}},
        (struct parameter) { .type = FUNCTION_TYPE_,   .value = { .function = { .function_id = hotcall_ecall_greater_than_two, .params = function_parameters, .n_params = 1 }}},
        (struct parameter) { .type = VARIABLE_TYPE_,   .value = { .variable = { .arg = &b, .fmt = 'b', .iter = false }}}
    );

    HCALL(
        ((struct hotcall_function_config) {
            .function_id = hotcall_ecall_plus_one,
            .has_return = false
        }),
        { .type = VARIABLE_TYPE_, .value = { .variable = { .arg = &x, .fmt = 'd' }}}
    );
    HCALL(
        ((struct hotcall_function_config) {
            .function_id = hotcall_ecall_plus_one,
            .has_return = false
        }),
        { .type = VARIABLE_TYPE_, .value = { .variable = { .arg = &x, .fmt = 'd' }}}
    );

    END_WHILE(sm_ctx);

    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();

    ASSERT_EQ(x, 4);
}



TEST(while, 2) {
    // Contract: the loop should execute 10 times and hence x should be 10 after the loop has terminated.
    hotcall_test_setup();
    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    int x = 0, y = 10;

    hotcall_bundle_begin(sm_ctx);
    unsigned int n_params = 1;
    BEGIN_WHILE(
        ((struct while_config) {
            .predicate_fmt = "d<d"
        }),
        (struct parameter) { .type = VARIABLE_TYPE_,   .value = { .variable = { .arg = &x, .fmt = 'd' }}},
        (struct parameter) { .type = VARIABLE_TYPE_,   .value = { .variable = { .arg = &y, .fmt = 'd' }}}
    );
    HCALL(
        ((struct hotcall_function_config) {
            .function_id = hotcall_ecall_plus_one,
            .has_return = false
        }),
        { .type = VARIABLE_TYPE_, .value = { .variable = { .arg = &x, .fmt = 'd' }}}
    );
    END_WHILE(sm_ctx);
    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();

    ASSERT_EQ(x, 10);
}

TEST(while, 3) {
    // Contract: the loop should execute 10 times and hence x should be 10 after the loop has terminated.
    hotcall_test_setup();
    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    int x = 0, y = 10, z = 0, counter = 0;
    hotcall_bundle_begin(sm_ctx);
    unsigned int n_params = 1;

    struct while_config while_config = {
        .predicate_fmt = "d<d"
    };
    BEGIN_WHILE(
        ((struct while_config) {
            .predicate_fmt = "d<d"
        }),
        (struct parameter) { .type = VARIABLE_TYPE_,   .value = { .variable = { .arg = &x, .fmt = 'd' }}},
        (struct parameter) { .type = VARIABLE_TYPE_,   .value = { .variable = { .arg = &y, .fmt = 'd' }}}
    );

    HCALL(
        ((struct hotcall_function_config) {
            .function_id = hotcall_ecall_plus_one,
            .has_return = false
        }),
        { .type = VARIABLE_TYPE_, .value = { .variable = { .arg = &x, .fmt = 'd' }}}
    );

    BEGIN_WHILE(
        ((struct while_config) {
            .predicate_fmt = "d<d"
        }),
        (struct parameter) { .type = VARIABLE_TYPE_,   .value = { .variable = { .arg = &z, .fmt = 'd' }}},
        (struct parameter) { .type = VARIABLE_TYPE_,   .value = { .variable = { .arg = &y, .fmt = 'd' }}}
    );

    HCALL(
        ((struct hotcall_function_config) {
            .function_id = hotcall_ecall_plus_one,
            .has_return = false
        }),
        { .type = VARIABLE_TYPE_, .value = { .variable = { .arg = &z, .fmt = 'd' }}}
    );
    HCALL(
        ((struct hotcall_function_config) {
            .function_id = hotcall_ecall_plus_one,
            .has_return = false
        }),
        { .type = VARIABLE_TYPE_, .value = { .variable = { .arg = &counter, .fmt = 'd' }}}
    );

    END_WHILE(sm_ctx);

    HCALL(
        ((struct hotcall_function_config) {
            .function_id = hotcall_ecall_zero,
            .has_return = false
        }),
        { .type = VARIABLE_TYPE_, .value = { .variable = { .arg = &z, .fmt = 'd' }}}
    );

    END_WHILE(sm_ctx);
    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();

    ASSERT_EQ(counter, 100);
}
