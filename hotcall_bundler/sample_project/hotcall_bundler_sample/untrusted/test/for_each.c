#include "gtest/gtest.h"
#include "test.h"
#include "hotcall-bundler-untrusted.h"
#include "functions.h"

#include "hotcall_for_each.h"



TEST(for_each,1) {
    //Contract: for each should apply the hcall on each element in the list
    hotcall_test_setup();

    BUNDLE_BEGIN();

    unsigned int n_params = 1, n_iters = 10;
    int xs[n_iters] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

    FOR_EACH(
        ((struct for_each_config) { .function_id = hotcall_ecall_plus_one }),
        VECTOR(xs, 'd', &n_iters)
    );

    BUNDLE_END();

    hotcall_test_teardown();
    for(int i = 0; i < n_iters; ++i) {
        ASSERT_EQ(xs[i], i + 1);
    }
}

TEST(for_each,2) {
    //Contract: for each should apply the hcall on each element in the list. All elements of xs should be 1 and all elements of ys 2.
    hotcall_test_setup();

    BUNDLE_BEGIN();

    unsigned int n_params = 1, n_iters = 10;
    int xs[n_iters] = { 0 };
    int ys[n_iters] = { 0 };

    FOR_EACH(
        ((struct for_each_config) { .function_id = hotcall_ecall_plus_plus }),
        VECTOR(xs, 'd', &n_iters),
        VECTOR(ys, 'd', &n_iters)
    );

    BUNDLE_END();

    hotcall_test_teardown();
    for(int i = 0; i < n_iters; ++i) {
        ASSERT_EQ(xs[i], 1);
        ASSERT_EQ(ys[i], 2);
    }
}

TEST(for_each,3) {
    //Contract: for each should apply the hcall on each element in the list. All elements of xs should be 5 in the end.
    hotcall_test_setup();

    BUNDLE_BEGIN();

    unsigned int n_iters = 10;
    int xs[n_iters] = { 0 };
    int y = 5;

    FOR_EACH(
        ((struct for_each_config) { .function_id = hotcall_ecall_plus_y }),
        VECTOR(xs, 'd', &n_iters),
        VAR(y, 'd')
    );

    BUNDLE_END();

    hotcall_test_teardown();
    for(int i = 0; i < n_iters; ++i) {
        ASSERT_EQ(xs[i], 5);
    }
}

TEST(for_each,4) {
    // Contract: for each should apply the hcall on each element in the list. All elements of xs should be 5 in the end. This test also tests giving a vector argumemnt as second parameter.
    hotcall_test_setup();

    BUNDLE_BEGIN();

    unsigned int n_params = 1, n_iters = 10;
    int xs[n_iters] = { 0 };
    int y = 5;

    FOR_EACH(
        ((struct for_each_config) { .function_id = hotcall_ecall_plus_y_v2 }),
        VAR(y, 'd'),
        VECTOR(xs, 'd', &n_iters)
    );

    BUNDLE_END();

    hotcall_test_teardown();

    for(int i = 0; i < n_iters; ++i) {
        ASSERT_EQ(xs[i], 5);
    }
}


TEST(for_each, 5) {
    /* Contract: */
    /*hotcall_test_setup();

    BUNDLE_BEGIN();

    unsigned int n_params = 1, n_iters = 3;

    struct A { int x; int y; int z; char c[32]; };
    //struct A as[n_iters] = { 0 };


    A a1 = { 0 }, a2 = { 0 }, a3 = { 0 };
    struct A *as[n_iters] = { &a1, &a2, &a3 };


    FOR_EACH(
        ((struct for_each_config) { .function_id = hotcall_ecall_plus_one }),
        VECTOR(as, 'p', &n_iters, .dereference = true, .access_member = offsetof(struct A, y)),
    );

    BUNDLE_END();

    hotcall_test_teardown();

    for(int i = 0; i < n_iters; ++i) {
        ASSERT_EQ(as[i]->y, 1);
        ASSERT_EQ(as[i]->x, 0);
    }*/
}
