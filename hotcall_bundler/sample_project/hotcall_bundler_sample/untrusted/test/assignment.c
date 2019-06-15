
#include "gtest/gtest.h"
#include "test.h"
#include "hotcall-untrusted.h"
#include "functions.h"


TEST(assign_var, 1) {
    // Copies the value of y to x and increments x by 1. The value of y should be unchanged.
    hotcall_test_setup();

    int x = 0;
    int y = 1;

    BUNDLE_BEGIN();

    ASSIGN_VAR(VAR(x, 'd'), VAR(y, 'd'));

    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(x, 2);
    ASSERT_EQ(y, 1);
}

TEST(assign_var, 2) {
    // Copies each value of xs to y in each iteration and increments the value of y by 1. After the for loop, y should have the value 9 + 1.
    // The list xs should be unmodified.
    hotcall_test_setup();
    unsigned int n_iters = 10;
    int xs[n_iters] = {0,1,2,3,4,5,6,7,8,9};
    int y = 0;

    BUNDLE_BEGIN();

    BEGIN_FOR({ .n_iters = &n_iters });

        ASSIGN_VAR(VAR(y, 'd'), VECTOR(xs, 'd'));
        HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(y, 'd'));

    END_FOR();

    BUNDLE_END();

    hotcall_test_teardown();

    for(int i = 0; i < n_iters; ++i) ASSERT_EQ(xs[i], i);

    ASSERT_EQ(y, 10);
}

TEST(assign_ptr, 1) {
    // Assigns the address of y to x and icrements the value x points to by 1.

    hotcall_test_setup();

    int *x = NULL;
    int y = 1;

    BUNDLE_BEGIN();

    ASSIGN_PTR(PTR(&x, 'd'), VAR(y, 'd'));

    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), PTR(&x, 'd', .dereference = true));

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(*x, 2);
    ASSERT_EQ(y, 2);
}
