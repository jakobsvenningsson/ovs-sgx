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

    //struct parameter vec[] = { VAR(xs, 'd'), VECTOR_v2(&vec[0], &n_iters) }, p = vec[1];

    FOR_EACH(((struct for_each_config) { .function_id = hotcall_ecall_plus_one, .n_iters = &n_iters}), VECTOR(xs, 'd'));

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

    FOR_EACH(((struct for_each_config) { .function_id = hotcall_ecall_plus_plus, .n_iters = &n_iters }), VECTOR(xs, 'd'), VECTOR(ys, 'd'));

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

    //struct parameter vec[] = { VAR(xs, 'd'), VECTOR_v2(&vec[0], &n_iters) }, p = vec[1];

    FOR_EACH(((struct for_each_config) { .function_id = hotcall_ecall_plus_y, .n_iters = &n_iters }), VECTOR(xs, 'd'), VAR(y, 'd'));

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

    //struct parameter vec[] = { VAR(xs, 'd'), VECTOR_v2(&vec[0], &n_iters) }, p = vec[1];

    FOR_EACH(
        ((struct for_each_config) { .function_id = hotcall_ecall_plus_y_v2, .n_iters = &n_iters }),
        VAR(y, 'd'),
        VECTOR(xs, 'd')
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



    /*struct parameter vec[] = {
        VAR(as, 'd'),
        STRUCT(&vec[0], .member_offset = offsetof(struct A, y)),
        PTR_v2(&vec[1], .dereference = true),
        VECTOR_v2(&vec[2], &n_iters)
    }, p = vec[3];*/

    struct parameter vec_param = VECTOR(as, 'd', &n_iters, .dereference = true, .member_offset = offsetof(struct A, y));

    FOR_EACH(((struct for_each_config) { .function_id = hotcall_ecall_plus_one, .n_iters = &n_iters }), vec_param);
    FOR_EACH(((struct for_each_config) { .function_id = hotcall_ecall_plus_one, .n_iters = &n_iters }), vec_param);

    BUNDLE_END();

    hotcall_test_teardown();

    for(int i = 0; i < n_iters; ++i) {
        ASSERT_EQ(as[i]->y, 2);
        ASSERT_EQ(as[i]->x, 0);
        ASSERT_EQ(as[i]->z, 0);
    }
}

TEST(for_each, 6) {
    /* Contract:  */

    hotcall_test_setup();

    unsigned int len = 3;
    int numbers[len] = { 0, 1, 2};

    BUNDLE_BEGIN();

    //struct parameter vec[] = { VAR(numbers, 'd'), VECTOR_v2(&vec[0], &len) }, p = vec[sizeof(vec)/sizeof(struct parameter) - 1];

    FOR_EACH(
        ((struct for_each_config) { .function_id = hotcall_ecall_plus_one, .n_iters = &len }), VECTOR(numbers, 'd', &len)
    );

    BUNDLE_END();

    hotcall_test_teardown();

    for(int i = 0; i < len; ++i) {
        ASSERT_EQ(numbers[i], i + 1);
    }
}


TEST(for_each, 7) {
    /* Contract:  */

    hotcall_test_setup();

    struct B {
        int a;
        int b;
    };

    struct A {
        int x;
        int y;
        struct B b;
    };

    A a1 = { 0 }, a2 = { 0 }, a3 = { 0 }, a4 = { 0 }, a5 = { 0 };

    unsigned int len = 5;
    void *as[len] = { &a1.b, &a2.b, &a3.b, &a4.b, &a5.b };

    BUNDLE_BEGIN();

    int offset = offsetof(struct A, b);
    //struct parameter vec_1[] = { VAR(as, 'p'), PTR_v2(&vec_1[0], .dereference = false), VECTOR_v2(&vec_1[1], &len) }, p1 = vec_1[2];

    FOR_EACH(
        ((struct for_each_config) { .function_id = hotcall_ecall_container_of, .n_iters = &len }), VECTOR(as, 'p', &len, .dereference = false), VAR(offset, 'd')
    );


    /*struct parameter vec_2[] = {
                                    VAR(as, 'd'),
                                    STRUCT(&vec_2[0], .member_offset = offsetof(struct A, y)),
                                    PTR_v2(&vec_2[1], .dereference = true),
                                    VECTOR_v2(&vec_2[2], &len)
                                }, p2 = vec_2[3];*/

    FOR_EACH(
        ((struct for_each_config) { .function_id = hotcall_ecall_plus_one, .n_iters = &len }), VECTOR(as, 'd', &len, .dereference = true, .member_offset = offsetof(struct A, y))
    );

    /*struct parameter vec_3[] = {
                                    VAR(as, 'd'),
                                    STRUCT(&vec_3[0], .member_offset = offsetof(struct A, x)),
                                    PTR_v2(&vec_3[1], .dereference = true),
                                    VECTOR_v2(&vec_3[2], &len)
                                }, p3 = vec_3[3];*/

    FOR_EACH(
        ((struct for_each_config) { .function_id = hotcall_ecall_plus_one, .n_iters = &len  }), VECTOR(as, 'd', &len, .dereference = true, .member_offset = offsetof(struct A, x))
    );
    BUNDLE_END();

    hotcall_test_teardown();


    for(int i = 0; i < len; ++i) {
        ASSERT_EQ(((struct A *) as[i])->x, 1);
        ASSERT_EQ(((struct A *) as[i])->y, 1);
    }
}

TEST(for_each, 8) {
    /* Contract:  */

    hotcall_test_setup();

    int x = 0, y = 1, z = 2;
    unsigned int len = 3;
    int *numbers[len] = { &x, &y, &z };

    BUNDLE_BEGIN();

    //struct parameter vec[] = { VAR(numbers, 'p'), PTR_v2(&vec[0], .dereference = true), VECTOR_v2(&vec[1], &len) }, p = vec[2];

    FOR_EACH(
        ((struct for_each_config) { .function_id = hotcall_ecall_plus_one, .n_iters = &len }), VECTOR(numbers, 'd', &len, .dereference = true)
    );

    BUNDLE_END();

    hotcall_test_teardown();


    for(int i = 0; i < len; ++i) {
        ASSERT_EQ(*numbers[i], i + 1);
    }
}
