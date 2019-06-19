
#include "gtest/gtest.h"
#include "test.h"
#include "hotcall-bundler-untrusted.h"
#include "functions.h"

#define MEMBER(TYPE, MEMBER) offsetof(TYPE, MEMBER)

TEST(struct_member_access, 1) {
    /* Contract:  */

    hotcall_test_setup();

    struct A {
        int x;
        int y;
    };

    A a = { 0 };

    BUNDLE_BEGIN();

    struct parameter vec1[] = { VAR(a, 'd'), STRUCT(&vec1[0], .member_offset = MEMBER(struct A, y))}, p1 = vec1[1];
    struct parameter vec2[] = { VAR(a, 'd'), STRUCT(&vec2[0], .member_offset = MEMBER(struct A, x))}, p2 = vec2[1];

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(a.x, 2);
    ASSERT_EQ(a.y, 1);
}

TEST(struct_member_access, 2) {
    /* Contract:  */

    /*hotcall_test_setup();

    struct A {
        int x;
        int y;
    };

    A a = { 0 };

    BUNDLE_BEGIN();

    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), PTR(&a, 'd', .dereference = false, .access_member = MEMBER(struct A, y)));
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), PTR(&a, 'd', .dereference = false, .access_member = MEMBER(struct A, x)));
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), PTR(&a, 'd', .dereference = false, .access_member = MEMBER(struct A, x)));

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(a.x, 2);
    ASSERT_EQ(a.y, 1);*/
}

TEST(struct_member_access, 3) {
    /* Contract:  */

    /*hotcall_test_setup();

    struct A {
        int x;
        int y;
    };

    A a1 = { 0 }, a2 = { 0 }, a3 = { 0 }, a4 = { 0 }, a5 = { 0 };

    unsigned int len = 5;
    struct A *as[len] = { &a1, &a2, &a3, &a4, &a5 };
    void *a_ptr = NULL;

    BUNDLE_BEGIN();

    BEGIN_FOR((struct for_config) {
        .n_iters = &len
    });

    ASSIGN_PTR(PTR(&a_ptr, 'd'), VECTOR(as, 'p', &len));

    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), PTR(&a_ptr, 'd', .dereference = true));

    END_FOR();

    BUNDLE_END();

    hotcall_test_teardown();


    for(int i = 0; i < len; ++i) {
        ASSERT_EQ(as[i]->x, 1);
        ASSERT_EQ(as[i]->y, 0);
    }*/
}
