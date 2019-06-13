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

    HCALL(CONFIG({ .function_id = hotcall_ecall_always_false, .has_return = true }), VAR(&res1, 'b'));
    HCALL(CONFIG({ .function_id = hotcall_ecall_always_true, .has_return = true }), VAR(&res2, 'b'));
    HCALL(CONFIG({ .function_id = hotcall_ecall_always_true, .has_return = true }), VAR(&res3, 'b'));
    IF(
        ((struct if_config) {
            .then_branch_len = 1,
            .else_branch_len = 2,
            .predicate_fmt = "!b&(b|b)&b",
            .return_if_false = false
        }),
        PTR(ptr), VAR(&res1, 'b'), VAR(&res2, 'b'), VAR(&res3, 'b')
    );
    THEN
        HCALL(CONFIG({ .function_id = hotcall_ecall_plus_one }), VAR(&x, 'd'));
    ELSE
        HCALL(CONFIG({ .function_id = hotcall_ecall_plus_one }), VAR(&x, 'd'));
        HCALL(CONFIG({ .function_id = hotcall_ecall_plus_one }), VAR(&x, 'd'));
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
    HCALL(CONFIG({ .function_id = hotcall_ecall_always_false, .has_return = true }), VAR(&res, 'b'));
    IF(
        INIT_IF_CONF(1, 2, "b|!b", false),
        VAR(&res, 'b'),
        FUNC(hotcall_ecall_always_true, NULL)
    );
    THEN
        HCALL(CONFIG({ .function_id = hotcall_ecall_plus_one }), VAR(&x, 'd'));
    ELSE
        HCALL(CONFIG({ .function_id = hotcall_ecall_plus_one }), VAR(&x, 'd'));
        HCALL(CONFIG({ .function_id = hotcall_ecall_plus_one }), VAR(&x, 'd'));
    END

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
    HCALL(CONFIG({ .function_id = hotcall_ecall_always_false, .has_return = true }), VAR(&res, 'b'));
    int a = 6, b = 3;
    struct parameter function_parameters[] = { VAR(&a, 'd'), VAR(&b, 'd') };
    IF(
        ((struct if_config) {
            .then_branch_len = 1,
            .else_branch_len = 2,
            .predicate_fmt = "b|!b",
            .return_if_false = true
        }),
        VAR( &res, 'b' ), FUNC( .function_id = hotcall_ecall_greater_than_y, .params = function_parameters, .n_params = 2 )
    );
    THEN
        HCALL(CONFIG({ .function_id = hotcall_ecall_plus_one }), VAR(&x, 'd'));
        HCALL(CONFIG({ .function_id = hotcall_ecall_plus_one }), VAR(&x, 'd'));
        HCALL(CONFIG({ .function_id = hotcall_ecall_plus_one }), VAR(&x, 'd'));
    ELSE
        HCALL(CONFIG({ .function_id = hotcall_ecall_plus_one }), VAR(&x, 'd'));
        HCALL(CONFIG({ .function_id = hotcall_ecall_plus_one }), VAR(&x, 'd'));
    END
    
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
    HCALL(CONFIG({ .function_id = hotcall_ecall_always_true }), VAR(&res, 'b'));

    IF(
        ((struct if_config) {
            .then_branch_len = 1,
            .else_branch_len = 0,
            .predicate_fmt = "b",
            .return_if_false = false
        }),
        VAR(&res, 'b')
    );
    THEN
        HCALL(CONFIG({ .function_id = hotcall_ecall_plus_one }), VAR(&x, 'd'));
    END

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
        ((struct if_config) {
            .then_branch_len = 1,
            .else_branch_len = 0,
            .predicate_fmt = "b",
            .return_if_false = false
        }),
        FUNC(.function_id = hotcall_ecall_always_true, .params = NULL)
    );
    THEN
        HCALL(CONFIG({ .function_id = hotcall_ecall_plus_one }), VAR(&x, 'd'));


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
        ((struct if_config) {
            .then_branch_len = 1,
            .else_branch_len = 0,
            .predicate_fmt = "!b",
            .return_if_false = false
        }),
        FUNC(.function_id = hotcall_ecall_always_false, .params =  NULL)
    );
    THEN
        HCALL(CONFIG({ .function_id = hotcall_ecall_plus_one }), VAR(&x, 'd'));


    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();

    ASSERT_EQ(x, 1);
}
