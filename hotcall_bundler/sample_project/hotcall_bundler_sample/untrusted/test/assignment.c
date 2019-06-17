
#include "gtest/gtest.h"
#include "test.h"
#include "hotcall-bundler-untrusted.h"
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

    /*hotcall_test_setup();

    int *x = NULL;
    int y = 1;

    BUNDLE_BEGIN();

    ASSIGN_PTR(PTR(&x, 'd'), VAR(y, 'd'));

    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), PTR(&x, 'd', .dereference = true));

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(*x, 2);
    ASSERT_EQ(y, 2);*/
}

TEST(assign_ptr, 2) {
    /* Contract: */

    /*hotcall_test_setup();

    int *x = NULL;
    int y = 1;

    BUNDLE_BEGIN();

    ASSIGN_PTR(PTR(&x, 'd'), PTR(&y, 'd'));

    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), PTR(&x, 'd', .dereference = true));

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(*x, 2);
    ASSERT_EQ(y, 2);*/
}

TEST(assign_ptr, 3) {
    /* Cointract:  */
    /*hotcall_test_setup();
    unsigned int n_iters = 10;
    int xs[n_iters] = {0,1,2,3,4,5,6,7,8,9};
    int *y_ptr = NULL;

    BUNDLE_BEGIN();

    BEGIN_FOR({ .n_iters = &n_iters });

        ASSIGN_PTR(PTR(&y_ptr, 'p'), VECTOR(xs, 'd', &n_iters));
        HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), PTR(&y_ptr, 'd', .dereference = true));

    END_FOR();

    BUNDLE_END();

    hotcall_test_teardown();

    for(int i = 0; i < n_iters; ++i) ASSERT_EQ(xs[i], i + 1);*/
}

/*TEST(assign_ptr, 4) {
    hotcall_test_setup();
    unsigned int n_iters = 3;
    struct A { int x; int y; };
    A a1 = { 0 }, a2 = { 0 }, a3 = { 0 };
    struct A *as[n_iters] = { &a1, &a2, &a3 };


    void *a_ptr = NULL;

    BUNDLE_BEGIN();

    BEGIN_FOR({ .n_iters = &n_iters });

        ASSIGN_PTR(PTR(&a_ptr, 'p'), VECTOR(as, 'p', &n_iters));
        HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), PTR(&a_ptr, 'p', .dereference = true, .access_member = offsetof(struct A, x)));

    END_FOR();

    BUNDLE_END();

    hotcall_test_teardown();

    //((struct A *) ((void * ) a_ptr))->x = 1;
    ASSERT_EQ(as[n_iters - 1]->x, 1);

    //for(int i = 0; i < n_iters - 1; ++i) ASSERT_EQ(as[i]->x, 0);
    //ASSERT_EQ(as[n_iters - 1]->x, 1);
}*/



/* Cointract:  */


TEST(assign_ptr, 4) {
    /*hotcall_test_setup();
    unsigned int n_iters = 3;
    struct A { int x; int y; };
    A a1 = { 0 }, a2 = { 0 }, a3 = { 0 };
    struct A *as[n_iters] = { &a1, &a2, &a3 };
    void *a_ptr = NULL;


    BUNDLE_BEGIN();

    BEGIN_FOR({ .n_iters = &n_iters });
        ASSIGN_PTR(PTR(&a_ptr, 'p'), VECTOR(as, 'p', &n_iters));
        HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), PTR(&a_ptr, 'p', .dereference = true, .access_member = offsetof(struct A, x)));

    END_FOR();

    BUNDLE_END();

    hotcall_test_teardown();

    for(int i = 0; i < n_iters; ++i) {
        ASSERT_EQ(as[i]->x, 1);
        ASSERT_EQ(as[i]->y, 0);
    }*/
}
