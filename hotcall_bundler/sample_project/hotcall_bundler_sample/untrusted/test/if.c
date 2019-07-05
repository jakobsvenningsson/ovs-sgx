#include "gtest/gtest.h"
#include "test.h"
#include "hotcall-bundler-untrusted.h"
#include "functions.h"

#include "hotcall_if.h"
#include "hotcall.h"
#include "postfix_translator.h"


TEST(if,1) {
    // Contract: a true condition should execute the "then" branch
    hotcall_test_setup();

    int x = 0;
    bool res1, res2, res3;
    bool *ptr = NULL;

    BUNDLE_BEGIN();

    HCALL(CONFIG( .function_id = hotcall_ecall_always_false, .has_return = true ), VAR(res1, 'b'));
    HCALL(CONFIG( .function_id = hotcall_ecall_always_true, .has_return = true ), VAR(res2, 'b'));
    HCALL(CONFIG( .function_id = hotcall_ecall_always_true, .has_return = true ), VAR(res3, 'b'));
    IF(
        ((struct if_config) { .predicate_fmt = "!b&(b|b)&b" }),
        PTR(ptr), VAR(res1, 'b'), VAR(res2, 'b'), VAR(res3, 'b')
    );
    THEN
        HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(x, 'd'));
    ELSE
        HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(x, 'd'));
        HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(x, 'd'));
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

    bool b;
    struct parameter function_parameters[] = { VAR(b, 'b') };
    HCALL(CONFIG( .function_id = hotcall_ecall_always_false, .has_return = true ), VAR(res, 'b'));
    IF(
        ((struct if_config) { .predicate_fmt = "b|!b" }),
        VAR(res, 'b'),
        FUNC(hotcall_ecall_always_true, .params = function_parameters, .n_params = 1)
    );
    THEN
        HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(x, 'd'));
    ELSE
        HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(x, 'd'));
        HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(x, 'd'));
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
    struct parameter function_parameters[] = { VAR(a, 'd'), VAR(b, 'd') };

    BUNDLE_BEGIN();

    HCALL(CONFIG( .function_id = hotcall_ecall_always_false, .has_return = true ), VAR(res, 'b'));
    IF(
        ((struct if_config) { .predicate_fmt = "b|!b" }),
        VAR(res, 'b' ), FUNC( .function_id = hotcall_ecall_greater_than_y, .params = function_parameters, .n_params = 2 )
    );
    THEN
        HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(x, 'd'));
        HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(x, 'd'));
        HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(x, 'd'));
        HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(x, 'd'));
    ELSE
        HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(x, 'd'));
        HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(x, 'd'));
        RETURN;
    END

    HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(x, 'd'));

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

    HCALL(CONFIG( .function_id = hotcall_ecall_always_true, .has_return = true ), VAR(res, 'b'));
    IF(
        ((struct if_config) { .predicate_fmt = "b" }),
        VAR(res, 'b')
    );
    THEN
        HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(x, 'd'));
    END

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(x, 1);
}



TEST(if,5) {
    hotcall_test_setup();

    int x = 0;

    BUNDLE_BEGIN();

    bool b;
    struct parameter function_parameters[] = { VAR(b, 'b') };

    IF(
        ((struct if_config) { .predicate_fmt = "b" }),
        FUNC(.function_id = hotcall_ecall_always_true, .params = function_parameters, .n_params = 1)
    );
    THEN
        HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));
    END

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(x, 1);
}

TEST(if,6) {
    hotcall_test_setup();

    int x = 0;

    BUNDLE_BEGIN();


    bool b;
    struct parameter function_parameters[] = { VAR(b, 'b') };

    IF(
        ((struct if_config) { .predicate_fmt = "!b" }),
        FUNC(.function_id = hotcall_ecall_always_false, .params = function_parameters, .n_params = 1)
    );
    THEN
        HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(x, 'd'));
    END

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(x, 1);
}

TEST(if,7) {
    hotcall_test_setup();

    int x = 0;

    BUNDLE_BEGIN();

    bool b;
    struct parameter function_parameters[] = { VAR(b, 'b') };

    IF(
        ((struct if_config) { .predicate_fmt = "!b" }),
        FUNC(.function_id = hotcall_ecall_always_false, .params =  function_parameters, .n_params = 1)
    );
    THEN
        RETURN;
    END

    HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(x, 'd'));

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(x, 0);
}

TEST(if,8) {
    hotcall_test_setup();

    int x = 0, y = 0, z = 0;

    BUNDLE_BEGIN();

    IF(((struct if_config) { .predicate_fmt = "!d" }), VAR(x, 'd'));
    THEN
    ELSE
    END
    HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(x, 'd'));

    IF(((struct if_config) { .predicate_fmt = "d" }), VAR(y, 'd'));
    THEN
    ELSE
    END
    HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(y, 'd'));

    IF(((struct if_config) { .predicate_fmt = "d" }), VAR(x, 'd'));
    THEN
    END
    HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(z, 'd'));

    IF(((struct if_config) { .predicate_fmt = "!d" }), VAR(x, 'd'));
    THEN
    END
    HCALL(CONFIG( .function_id = hotcall_ecall_plus_one ), VAR(z, 'd'));

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(x, 1);
    ASSERT_EQ(y, 1);
    ASSERT_EQ(z, 2);

}


TEST(if,9) {
    hotcall_test_setup();

    struct R {
        int dummy1;
        uint32_t hard_timeout;
        char dummy2[32];
        uint32_t idle_timeout;
    };

    int is_eviction_enabled = 1;

    struct R r1 = { 0, 123, "", 0 };
    struct R r2 = { 1, 0, "", 0 };

    unsigned int len = 2;
    struct R *rs[len] = { &r1, &r2 };

    int xs[len] = { 0 };

    BUNDLE_BEGIN();

    BEGIN_FOR((struct for_config) {
        .n_iters = &len
    });

        IF(
            ((struct if_config) { .predicate_fmt = "d&(u|u)" }),
            VAR(is_eviction_enabled, 'd'),
            VECTOR(rs, 'u', &len, .dereference = true, offsetof(struct R, hard_timeout)),
            VECTOR(rs, 'u', &len, .dereference = true, offsetof(struct R, idle_timeout))
        );
        THEN
            HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VECTOR(xs, 'd'));
        END

    END_FOR();

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(xs[0], 1);
    ASSERT_EQ(xs[1], 0);
}

TEST(if, 10) {
    /* Contract:  RETURN should stop execution of the bundle it appears inside of. */

    hotcall_test_setup();

    int x = 0, y = 1;

    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));

    printf("X: %d\n", x);

    BUNDLE_BEGIN();

    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));

    RETURN;

    BUNDLE_END();

    BUNDLE_BEGIN();

    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));
    IF(((struct if_config) { .predicate_fmt = "d" }), VAR(y, 'd'));
    THEN
        RETURN;
    END

    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));

    BUNDLE_END();

    BUNDLE_BEGIN();

    RETURN;

    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));

    BUNDLE_END();

    BUNDLE_BEGIN();
    if(y) {
        HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));
        HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));
        HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));
        HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));
    }
    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(x, 8);
}

TEST(if,12) {

    hotcall_test_setup();

    BUNDLE_BEGIN();

    unsigned int n_iters = 10;
    int xs[n_iters] = { 0 };
    int x = 0;
    bool always_true;
    HCALL(CONFIG(.function_id = hotcall_ecall_always_true, .has_return = true), VAR(always_true, 'b'));

    IF(
        ((struct if_config) { .predicate_fmt = "b" }), VAR(always_true, 'b')
    );
    THEN
        BEGIN_FOR((struct for_config) {
            .n_iters = &n_iters
        });
            IF(
                ((struct if_config) { .predicate_fmt = "b" }),
                VAR(always_true, 'b')
            );
            THEN
                HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));
                HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VECTOR(xs, 'd'));
            END
        END_FOR();
    END

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(x, n_iters);
    for(int i = 0; i < n_iters; ++i) ASSERT_EQ(xs[i], 1);
}

TEST(if,13) {

    /*hotcall_test_setup();

    BUNDLE_BEGIN();

    int x = 0;

    BUNDLE_IF_TRUE(NULL);

    BUNDLE_IF_ELSE();

    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));

    BUNDLE_IF_END();

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(x, 5);*/
}

TEST(if,14) {

    /*hotcall_test_setup();

    BUNDLE_BEGIN();

    int x = 0;

    BUNDLE_IF_TRUE(true);

    BUNDLE_IF_ELSE();

    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));

    BUNDLE_IF_END();

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(x, 0);*/
}
