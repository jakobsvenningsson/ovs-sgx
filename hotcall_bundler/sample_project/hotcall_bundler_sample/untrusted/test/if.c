#include "gtest/gtest.h"
#include "test.h"
#include "hotcall-untrusted.h"
#include "functions.h"

#include "hotcall_if.h"
#include "hotcall.h"

TEST(if,1) {
    // Contract: a true condition should execute the "then" branch
    hotcall_test_setup();
    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    int x = 0;
    hotcall_bundle_begin(sm_ctx);
    bool res1, res2, res3;
    bool *ptr = NULL;

    HCALL_1(
        sm_ctx,
        CONFIG({ .f_id = hotcall_ecall_always_false, .has_return = true }),
        VARIABLE_PARAM(&res1, 'b')
    );
    HCALL_1(
        sm_ctx,
        CONFIG({ .f_id = hotcall_ecall_always_true, .has_return = true }),
        VARIABLE_PARAM(&res2, 'b')
    );
    HCALL_1(
        sm_ctx,
        CONFIG({ .f_id = hotcall_ecall_always_true, .has_return = true }),
        VARIABLE_PARAM(&res3, 'b')
    );

    IF(
        sm_ctx,
        ((struct if_config) {
            .then_branch_len = 1,
            .else_branch_len = 2,
            .predicate_fmt = "!b&(b|b)&b",
            .return_if_false = false
        }),
        (struct parameter) { .type = POINTER_TYPE_,   .value = { .variable = { .arg = ptr, .fmt = 'b', .iter = false }}},
        (struct parameter) { .type = VARIABLE_TYPE_,  .value = { .variable = { .arg = &res1, .fmt = 'b', .iter = false }}},
        (struct parameter) { .type = VARIABLE_TYPE_,  .value = { .variable = { .arg = &res2, .fmt = 'b', .iter = false }}},
        (struct parameter) { .type = VARIABLE_TYPE_,  .value = { .variable = { .arg = &res3, .fmt = 'b', .iter = false }}}
    );
    THEN
        HCALL_1(
            sm_ctx,
            CONFIG({ .f_id = hotcall_ecall_plus_one, .has_return = false }),
            VARIABLE_PARAM(&x, 'd')
        );
    ELSE
        HCALL_1(
            sm_ctx,
            CONFIG({ .f_id = hotcall_ecall_plus_one, .has_return = false }),
            VARIABLE_PARAM(&x, 'd')
        );
        HCALL_1(
            sm_ctx,
            CONFIG({ .f_id = hotcall_ecall_plus_one, .has_return = false }),
            VARIABLE_PARAM(&x, 'd')
        );
    END

    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();

    ASSERT_EQ(x, 1);
}



TEST(if,2) {
    // Contract: a false condition should execute the "else" branch
    hotcall_test_setup();

    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    int x = 0;
    hotcall_bundle_begin(sm_ctx);

    bool res;
    HCALL_1(
        sm_ctx,
        CONFIG({ .f_id = hotcall_ecall_always_false, .has_return = true }),
        VARIABLE_PARAM(&res, 'b')
    );
    IF(
        sm_ctx,
        INIT_IF_CONF(1, 2, "b|!b", false),
        (struct parameter) { .type = VARIABLE_TYPE_, .value = { .variable = { .arg = &res, .fmt = 'b' }}},
        (struct parameter) { .type = FUNCTION_TYPE_, .value = { .function = { .f_id = hotcall_ecall_always_true, .params = NULL}}}
    );
    THEN
        HCALL_1(
            sm_ctx,
            CONFIG({ .f_id = hotcall_ecall_plus_one, .has_return = false }),
            VARIABLE_PARAM(&x, 'd')
        );
    ELSE
        HCALL_1(
            sm_ctx,
            CONFIG({ .f_id = hotcall_ecall_plus_one, .has_return = false }),
            VARIABLE_PARAM(&x, 'd')
        );
        HCALL_1(
            sm_ctx,
            CONFIG({ .f_id = hotcall_ecall_plus_one, .has_return = false }),
            VARIABLE_PARAM(&x, 'd')
        );


    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();

    ASSERT_EQ(x, 2);
}

TEST(if,3) {
    // Contract: a false condition and "return_if_false = true" shall stop execution after else branch.
    hotcall_test_setup();

    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    int x = 0;
    hotcall_bundle_begin(sm_ctx);

    bool res;
    HCALL_1(
        sm_ctx,
        CONFIG({ .f_id = hotcall_ecall_always_false, .has_return = true }),
        VARIABLE_PARAM(&res, 'b')
    );

    int a = 6, b = 3;
    struct parameter function_parameters[] = {
        { .type = VARIABLE_TYPE_, .value = { .variable = { .arg = &a, .fmt = 'd' }}},
        { .type = VARIABLE_TYPE_, .value = { .variable = { .arg = &b, .fmt = 'd' }}}
    };

    IF(
        sm_ctx,
        ((struct if_config) {
            .then_branch_len = 1,
            .else_branch_len = 2,
            .predicate_fmt = "b|!b",
            .return_if_false = true
        }),
        (struct parameter) { .type = VARIABLE_TYPE_,
                            .value = { .variable = { .arg = &res, .fmt = 'b' }}},
        (struct parameter) { .type = FUNCTION_TYPE_,
                            .value = { .function = { .f_id = hotcall_ecall_greater_than_y, .params = function_parameters, .n_params = 2}}}
    );
    THEN
        HCALL_1(
            sm_ctx,
            CONFIG({ .f_id = hotcall_ecall_plus_one, .has_return = false }),
            VARIABLE_PARAM(&x, 'd')
        );
        HCALL_1(
            sm_ctx,
            CONFIG({ .f_id = hotcall_ecall_plus_one, .has_return = false }),
            VARIABLE_PARAM(&x, 'd')
        );
        HCALL_1(
            sm_ctx,
            CONFIG({ .f_id = hotcall_ecall_plus_one, .has_return = false }),
            VARIABLE_PARAM(&x, 'd')
        );
    ELSE
        HCALL_1(
            sm_ctx,
            CONFIG({ .f_id = hotcall_ecall_plus_one, .has_return = false }),
            VARIABLE_PARAM(&x, 'd')
        );
        HCALL_1(
            sm_ctx,
            CONFIG({ .f_id = hotcall_ecall_plus_one, .has_return = false }),
            VARIABLE_PARAM(&x, 'd')
        );


    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();

    ASSERT_EQ(x, 2);
}

TEST(if,4) {
    hotcall_test_setup();
    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    int x = 0;
    hotcall_bundle_begin(sm_ctx);
    bool res;
    HCALL_1(
        sm_ctx,
        CONFIG({ .f_id = hotcall_ecall_always_true, .has_return = true }),
        VARIABLE_PARAM(&res, 'b')
    );

    IF(
        sm_ctx,
        ((struct if_config) {
            .then_branch_len = 1,
            .else_branch_len = 0,
            .predicate_fmt = "b",
            .return_if_false = false
        }),
        (struct parameter) { .type = VARIABLE_TYPE_,  .value = { .variable = { .arg = &res, .fmt = 'b' }}},
    );
    THEN
        HCALL_1(
            sm_ctx,
            CONFIG({ .f_id = hotcall_ecall_plus_one, .has_return = false }),
            VARIABLE_PARAM(&x, 'd')
        );


    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();

    ASSERT_EQ(x, 1);
}



TEST(if,5) {
    hotcall_test_setup();
    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    int x = 0;
    hotcall_bundle_begin(sm_ctx);
    IF(
        sm_ctx,
        ((struct if_config) {
            .then_branch_len = 1,
            .else_branch_len = 0,
            .predicate_fmt = "b",
            .return_if_false = false
        }),
        (struct parameter) { .type = FUNCTION_TYPE_,  .value = { .function = { .f_id = hotcall_ecall_always_true, .params =  NULL }}}
    );
    THEN
        HCALL_1(
            sm_ctx,
            CONFIG({ .f_id = hotcall_ecall_plus_one, .has_return = false }),
            VARIABLE_PARAM(&x, 'd')
        );


    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();

    ASSERT_EQ(x, 1);
}

TEST(if,6) {
    hotcall_test_setup();
    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    int x = 0;
    hotcall_bundle_begin(sm_ctx);
    IF(
        sm_ctx,
        ((struct if_config) {
            .then_branch_len = 1,
            .else_branch_len = 0,
            .predicate_fmt = "!b",
            .return_if_false = false
        }),
        (struct parameter) { .type = FUNCTION_TYPE_,  .value = { .function = { .f_id = hotcall_ecall_always_false, .params =  NULL }}}
    );
    THEN
        HCALL_1(
            sm_ctx,
            CONFIG({ .f_id = hotcall_ecall_plus_one, .has_return = false }),
            VARIABLE_PARAM(&x, 'd')
        );


    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();

    ASSERT_EQ(x, 1);
}
