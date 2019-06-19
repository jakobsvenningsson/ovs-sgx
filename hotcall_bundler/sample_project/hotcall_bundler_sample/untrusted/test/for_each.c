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

    struct parameter vec[] = { VAR(xs, 'd'), VECTOR_v2(&vec[0], &n_iters) }, p = vec[1];

    FOR_EACH(((struct for_each_config) { .function_id = hotcall_ecall_plus_one }), p);

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

    unsigned int n_iters = 10;
    int xs[n_iters] = { 0 };
    int ys[n_iters] = { 0 };

    struct parameter vec1[] = { VAR(xs, 'd'), VECTOR_v2(&vec1[0], &n_iters) }, p1 = vec1[1];
    struct parameter vec2[] = { VAR(ys, 'd'), VECTOR_v2(&vec2[0], &n_iters) }, p2 = vec2[1];

    FOR_EACH(((struct for_each_config) { .function_id = hotcall_ecall_plus_plus }), p1, p2);

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

    struct parameter vec[] = { VAR(xs, 'd'), VECTOR_v2(&vec[0], &n_iters) }, p = vec[1];

    FOR_EACH(((struct for_each_config) { .function_id = hotcall_ecall_plus_y }), p, VAR(y, 'd'));

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

    struct parameter vec[] = { VAR(xs, 'd'), VECTOR_v2(&vec[0], &n_iters) }, p = vec[1];

    FOR_EACH(
        ((struct for_each_config) { .function_id = hotcall_ecall_plus_y_v2 }),
        VAR(y, 'd'),
        p
    );

    BUNDLE_END();

    hotcall_test_teardown();

    for(int i = 0; i < n_iters; ++i) {
        ASSERT_EQ(xs[i], 5);
    }
}


TEST(for_each, 5) {
    /* Contract: */
    hotcall_test_setup();

    BUNDLE_BEGIN();

    unsigned int n_params = 1, n_iters = 3;

    struct A { int x; int z; char c[32]; int y; };

    A a1 = { 0 }, a2 = { 0 }, a3 = { 0 };
    struct A *as[n_iters] = { &a1, &a2, &a3 };



    struct parameter vec[] = {
        VAR(as, 'd'),
        STRUCT(&vec[0], .member_offset = offsetof(struct A, y)),
        PTR_v2(&vec[1], .dereference = true),
        VECTOR_v2(&vec[2], &n_iters)
    }, p = vec[3];

    FOR_EACH(((struct for_each_config) { .function_id = hotcall_ecall_plus_one }), p);
    FOR_EACH(((struct for_each_config) { .function_id = hotcall_ecall_plus_one }), p);

    BUNDLE_END();

    hotcall_test_teardown();

    for(int i = 0; i < n_iters; ++i) {
        ASSERT_EQ(as[i]->y, 2);
        ASSERT_EQ(as[i]->x, 0);
        ASSERT_EQ(as[i]->z, 0);
    }
}
