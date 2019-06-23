
#include "gtest/gtest.h"
#include "test.h"
#include "hotcall-bundler-untrusted.h"
#include "functions.h"

#define MEMBER(TYPE, MEMBER) offsetof(TYPE, MEMBER)

TEST(struct, 1) {
    /* Contract:  */

    hotcall_test_setup();

    struct A {
        int x;
        int y;
    };

    A a = { 0 };

    BUNDLE_BEGIN();

    //struct parameter vec1[] = { VAR(a, 'd'), STRUCT(&vec1[0], .member_offset = MEMBER(struct A, y))}, p1 = vec1[1];
    //struct parameter vec2[] = { VAR(a, 'd'), STRUCT(&vec2[0], .member_offset = MEMBER(struct A, x))}, p2 = vec2[1];

    //HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), STRUCT(a, .member_fmt = 'd', .member_offset = MEMBER(struct A, y)));
    //HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), STRUCT(a, .member_fmt = 'd'));
    //HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), STRUCT(a, .member_fmt = 'd', .member_offset = MEMBER(struct A, x)));

    //HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), p1);
    //HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), p2);
    //HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), p2);

    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(a, 'd', .member_offset = MEMBER(struct A, y)));
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(a, 'd'));
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(a, 'd', .member_offset = MEMBER(struct A, x)));

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(a.x, 2);
    ASSERT_EQ(a.y, 1);
}

TEST(struct, 2) {
    /* Contract:  */

    hotcall_test_setup();

    struct A {
        int x;
        int y;
    };

    A a = { 0 };
    struct A *a_ptr = &a;

    BUNDLE_BEGIN();

    /*struct parameter vec1[] = { VAR(a, 'd'),
                                STRUCT(&vec1[0], .member_offset = MEMBER(struct A, x)),
                                PTR_v2(&vec1[1], .dereference = false)
                              } , p1 = vec1[2];

    struct parameter vec2[] = { VAR(a_ptr, 'd'),
                                STRUCT(&vec2[0], .member_offset = MEMBER(struct A, y)),
                                PTR_v2(&vec2[1], .dereference = true)
                            }, p2 = vec2[2];*/


    /*struct parameter struct_p1 = STRUCT(a, .member_fmt = 'd', .member_offset = MEMBER(struct A, x));
    struct parameter struct_p2 = STRUCT(a, .member_fmt = 'd', .member_offset = MEMBER(struct A, y));

    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), PTR_v2(&struct_p1, .dereference = false));
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), PTR_v2(&struct_p2, .dereference = false));
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), PTR_v2(&struct_p2, .dereference = false));*/

    //HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), p1);
    //HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), p2);
    //HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), p2);

    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(a, 'd', .member_offset = MEMBER(struct A, x)));
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), PTR(a_ptr, 'd', .dereference = false, .member_offset = MEMBER(struct A, y)));
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), PTR(a_ptr, 'd', .dereference = false, .member_offset = MEMBER(struct A, y)));

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(a.x, 1);
    ASSERT_EQ(a.y, 2);
}

TEST(struct, 3) {
    /* Contract:  */

/*    hotcall_test_setup();

    struct A {
        int x;
        int y;
    };

    A a1 = { 0 }, a2 = { 0 }, a3 = { 0 }, a4 = { 0 }, a5 = { 0 };

    unsigned int len = 5;
    struct A as[len] = { a1, a2, a3, a4, a5 };*/

    //struct parameter p_struct = STRUCT(as, .member_fmt = 'd', .member_offset = offsetof(struct A, y), .struct_size = sizeof(struct A));


    /*struct parameter vec[] = { VAR(as, 'd'),
                                STRUCT(&vec[0], .member_offset = MEMBER(struct A, y), .struct_size = sizeof(struct A)),
                                VECTOR_v2(&vec[1], .len = &len)
                            }, p = vec[2];*/

    /*BUNDLE_BEGIN();

    BEGIN_FOR((struct for_config) {
        .n_iters = &len
    });

    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VECTOR(as, ''));

    END_FOR();

    BUNDLE_END();

    hotcall_test_teardown();*/


    /*for(int i = 0; i < len; ++i) {
        ASSERT_EQ(as[i].x, 0);
        ASSERT_EQ(as[i].y, 1);
    }*/
}

TEST(struct, 4) {
    /* Contract:  */

    hotcall_test_setup();

    struct A {
        int x;
        int y;
    };

    A a1 = { 0 }, a2 = { 0 }, a3 = { 0 }, a4 = { 0 }, a5 = { 0 };

    unsigned int len = 5;
    struct A *as[len] = { &a1, &a2, &a3, &a4, &a5 };


    /*struct parameter vec[] = {  VAR(as, 'd'),
                                STRUCT(&vec[0], .member_offset = MEMBER(struct A, y)),
                                PTR_v2(&vec[1], .dereference = true),
                                VECTOR_v2(&vec[2], .len = &len)
                            }, p = vec[3];


    struct parameter p_struct = STRUCT(as, .member_fmt = 'p', .member_offset = offsetof(struct A, y));
    struct parameter p_ptr = PTR_v2(&p_struct, .dereference = true);*/

    BUNDLE_BEGIN();

    BEGIN_FOR((struct for_config) {
        .n_iters = &len
    });

    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VECTOR(as, 'p', &len, .dereference = true, .member_offset = offsetof(struct A, y)));

    END_FOR();

    BUNDLE_END();

    hotcall_test_teardown();


    for(int i = 0; i < len; ++i) {
        ASSERT_EQ(as[i]->x, 0);
        ASSERT_EQ(as[i]->y, 1);
    }
}

/*
TEST(struct_, 10) {
     // Contract:

    hotcall_test_setup();

    struct B {
        int a;
        int b;
        int c;
    };

    struct A {
        struct B *x;
        int y;
    };

    A a1 = { 0 }, a2 = { 0 }, a3 = { 0 }, a4 = { 0 }, a5 = { 0 };
    B b1 = { 0 }, b2 = { 0 }, b3 = { 0 }, b4 = { 0 }, b5 = { 0 };
    a1.x = &b1;
    a2.x = &b2;
    a3.x = &b3;
    a4.x = &b4;
    a5.x = &b5;

    unsigned int len = 5;
    struct A *as[len] = { &a1, &a2, &a3, &a4, &a5 };

    int y = 5;


    struct parameter vec[] = {
                                VAR(as, 'd'),
                                STRUCT(&vec[0], .member_offset = MEMBER(struct B, c)),
                                PTR_v2(&vec[1], .dereference = true),
                                STRUCT(&vec[2], .member_offset = MEMBER(struct A, x)),
                                PTR_v2(&vec[3], .dereference = true),
                                VECTOR_v2(&vec[4], .len = &len)
                            }, p = vec[5];


    BUNDLE_BEGIN();

    BEGIN_FOR((struct for_config) {
        .n_iters = &len
    });

    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), p);
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_y), p, VAR(y, 'd'));

    END_FOR();

    BUNDLE_END();

    hotcall_test_teardown();

    for(int i = 0; i < len; ++i) {
        ASSERT_EQ(as[i]->x->c, 1 + 5);
        ASSERT_EQ(as[i]->y, 0);
        ASSERT_EQ(as[i]->x->a, 0);
        ASSERT_EQ(as[i]->x->b, 0);
    }
}*/

TEST(struct, 5) {
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

    BEGIN_FOR((struct for_config) {
        .n_iters = &len
    });
    int offset = offsetof(struct A, b);

    //struct parameter vec_1[] = { VAR(as, 'p'), PTR_v2(&vec_1[0], .dereference = false), VECTOR_v2(&vec_1[1], &len) };
    //struct parameter p_offset = VAR(offset, 'd');

    HCALL(CONFIG(.function_id = hotcall_ecall_container_of), VECTOR(as, 'p', &len, .dereference = false),  VAR(offset, 'd'));

    /*struct parameter vec_2[] = {
                                    VAR(as, 'd'),
                                    STRUCT(&vec_2[0], .member_offset = offsetof(struct A, y)),
                                    PTR_v2(&vec_2[1], .dereference = true),
                                    VECTOR_v2(&vec_2[2], &len)
                                };*/

    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VECTOR(as, 'p', &len, .dereference = true, .member_offset = offsetof(struct A, y)));

    END_FOR();

    BUNDLE_END();

    hotcall_test_teardown();


    for(int i = 0; i < len; ++i) {
        ASSERT_EQ(((struct A *) as[i])->x, 0);
        ASSERT_EQ(((struct A *) as[i])->y, 1);
    }
}



TEST(struct, 9) {
    /* Contract:  */

    hotcall_test_setup();

    struct A {
        int x; char dummy[32]; int y;
    };
    struct A a; struct A *a_ptr = &a;

    BUNDLE_BEGIN();

    //struct parameter vec[] = { VAR(a_ptr, 'd'), STRUCT(&vec[0], .member_offset = offsetof(struct A, y)), PTR_v2(&vec[1], .dereference = true) }, p = vec[2];

    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), PTR(a_ptr, 'd', .dereference = false, .member_offset = offsetof(struct A, y)));

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(a.y, 1);

}
