#include "gtest/gtest.h"
#include "test.h"
#include "hotcall-untrusted.h"
#include "functions.h"

#include "hotcall_for.h"

TEST(for,1) {
    //Contract: the hcalls inside the for loop should add 3 to each element in xs and 2 to each element in ys.
    hotcall_test_setup();
    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    unsigned int n_iters = 10;
    int xs[n_iters] = { 0 };
    int ys[n_iters] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    BUNDLE_BEGIN();

    BEGIN_FOR(((struct for_config) {
        .n_iters = &n_iters
    }));

        HCALL(CONFIG(.function_id = hotcall_ecall_plus_plus), VECTOR(xs, 'd'), VECTOR(ys, 'd'));
        HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VECTOR(xs, 'd'));
        HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VECTOR(xs, 'd'));

    END_FOR();

    BUNDLE_END();

    hotcall_test_teardown();

    for(int i = 0; i < n_iters; ++i) {
        ASSERT_EQ(xs[i], 3);
        ASSERT_EQ(ys[i], i + 2);
    }
}

TEST(for,2) {
    //Contract: the "then" branch should not be executed for the first 3 iterations of the loop.
    hotcall_test_setup();

    unsigned int n_params = 1, n_iters = 10;
    int xs[n_iters] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    bool ret;

    BUNDLE_BEGIN();

    BEGIN_FOR({ .n_iters = &n_iters });

        HCALL(CONFIG(.function_id = hotcall_ecall_greater_than_two, .has_return = true), VECTOR(xs, 'd'), VAR(&ret, 'b'));
        
        IF(
            ((struct if_config) { .predicate_fmt = "!b&b" }),
            PTR(NULL),
            VAR(&ret, 'b')
        );
        THEN
            HCALL(CONFIG(.function_id = hotcall_ecall_plus_one ), VECTOR(xs, 'd'));
        END

    END_FOR();

    BUNDLE_END();

    hotcall_test_teardown();

    for(int i = 0; i < n_iters; ++i) {
        if(i > 2) {
            ASSERT_EQ(xs[i], i + 1);
        } else {
            ASSERT_EQ(xs[i], i);
        }
    }
}
