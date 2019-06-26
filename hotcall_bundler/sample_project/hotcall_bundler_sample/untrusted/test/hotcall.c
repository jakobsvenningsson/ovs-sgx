#include "gtest/gtest.h"
#include "test.h"
#include "hotcall-bundler-untrusted.h"
#include "hotcall.h"
#include "functions.h"

TEST(hotcall, 1) {
    /* Contract: When a hotcall is executed outside a 'bundle' then it shall block and execute immediately. */
    hotcall_test_setup();

    int x = 0;
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));

    hotcall_test_teardown();

    ASSERT_EQ(x, 1);
}

TEST(hotcall, 2) {
    /* Contract: When a hotcall is executed inside a 'bundle' then the hotcall shall be buffered and the method call shall return immediately.
       When the bundle is closed, execution is blocked and all buffered hotcalls will be executed. */

    hotcall_test_setup();

    BUNDLE_BEGIN();

    int x = 0;
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));

    // x shall still be 0 since the previous hotcall has only been schedueled and buffered but not executed.
    ASSERT_EQ(x, 0);

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(x, 1);
}

TEST(hotcall, 3) {
    /* Contract: When a function is marked to return a value, the last parameter given as input to HCALL (in this case y)
       will be used to store the return value. */

    hotcall_test_setup();

    BUNDLE_BEGIN();

    int x = 0, y = 0;
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one_ret, .has_return = true ), VAR(x, 'd'), VAR(y, 'd'));

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(x, 0);
    ASSERT_EQ(y, 1);
}

TEST(hotcall, 4) {
    /* Contract: The return value of the first function is used as input in the second functtion. The variable z should hold the value of 2 after executing the bundle. */

    hotcall_test_setup();

    BUNDLE_BEGIN();

    int x = 0, y = 0, z = 0;
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one_ret, .has_return = true ), VAR(x, 'd'), VAR(y, 'd'));
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one_ret, .has_return = true ), VAR(y, 'd'), VAR(z, 'd'));

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(x, 0);
    ASSERT_EQ(y, 1);
    ASSERT_EQ(z, 2);

}

TEST(hotcall, 5) {
    /* Contract:  Equal test to above but using a pointer variable instead of a value variable.*/

    hotcall_test_setup();

    BUNDLE_BEGIN();

    int x = 0, y = 0, z = 0;
    int *x_ptr = &x, *y_ptr = &y;

    //struct parameter vec1[] = { VAR(x_ptr, 'd'), PTR(&vec1[0], .dereference = true) }, p1 = vec1[1];
    //struct parameter vec2[] = { VAR(y_ptr, 'd'), PTR(&vec2[0], .dereference = true) }, p2 = vec2[1];

    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one_ret, .has_return = true ), PTR(x_ptr), VAR(y, 'd'));
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one_ret, .has_return = true ), PTR(y_ptr), VAR(z, 'd'));

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(x, 0);
    ASSERT_EQ(y, 1);
    ASSERT_EQ(z, 2);

}

TEST(hotcall, 6) {
    /* Contract:  */

    hotcall_test_setup();

    BUNDLE_BEGIN();

    int x = 0, y = 1;
    int *x_ptr = &x;

    //struct parameter vec1[] = { VAR(x_ptr, 'd'), PTR_v2(&vec1[0], .dereference = false) }, p1 = vec1[1];
    //struct parameter vec2[] = { VAR(y, 'd'), PTR_v2(&vec2[0], .dereference = false) }, p2 = vec2[1];

    HCALL(CONFIG(.function_id = hotcall_ecall_change_ptr_to_ptr), PTR(&x_ptr), VAR(y));

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(*x_ptr, 1);
    ASSERT_EQ(y, 1);
}


TEST(hotcall, 7) {
    /* Contract:  */

    hotcall_test_setup();

    struct A {
        int x;
        int y;
    };

    A a = { 0 };

    struct parameter vec1[] = { VAR(a, 'd'), STRUCT(&vec1[0], .member_offset = offsetof(struct A, y)) }, p1 = vec1[1];
    struct parameter vec2[] = { VAR(a, 'd'), STRUCT(&vec2[0], .member_offset = offsetof(struct A, x)) }, p2 = vec2[1];

    BUNDLE_BEGIN();


    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(a, 'd', .dereference = false, .member_offset = offsetof(struct A, y)));
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(a, 'd', .dereference = false, .member_offset = offsetof(struct A, x)));
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(a, 'd', .dereference = false, .member_offset = offsetof(struct A, x)));

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(a.x, 2);
    ASSERT_EQ(a.y, 1);
}

TEST(hotcall, 8) {
    /* Contract:  */

    hotcall_test_setup();

    struct B {
        int x;
        int y;
    };

    struct A {
        int z;
        struct B b;
    };

    A a = { 0 };

    void *a_ptr = &a;

    //struct parameter vec1[] = { VAR(a_ptr, 'd'), STRUCT(&vec1[0], .member_offset = offsetof(struct B, y)), PTR_v2(&vec1[1], .dereference = true) }, p1 = vec1[2];

    BUNDLE_BEGIN();

    int b_offset = offsetof(struct A, b);

    HCALL(CONFIG(.function_id = hotcall_ecall_offset_of), PTR(&a_ptr), VAR(b_offset, 'u'));
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), PTR(&a_ptr, 'd', .dereference = true, .member_offset = offsetof(struct B, y)));

    BUNDLE_END();

    hotcall_test_teardown();


    ASSERT_EQ(((struct B *) a_ptr)->x, 0);
    ASSERT_EQ(((struct B *) a_ptr)->y, 1);

    ASSERT_EQ(a.z, 0);
    ASSERT_EQ(a.b.y, 1);
    ASSERT_EQ(a.b.x, 0);
}

TEST(hotcall, 9) {
    /* Contract:  */

    hotcall_test_setup();

    struct B {
        int x;
        int y;
    };

    struct A {
        int z;
        struct B b;
    };

    A a = { 0 };

    BUNDLE_BEGIN();

    void *ptr = &a.b;
    int b_offset = offsetof(struct A, b);

    //struct parameter vec1[] = { VAR(ptr, 'd'), STRUCT(&vec1[0], .member_offset = offsetof(struct A, z)), PTR_v2(&vec1[1], .dereference = true) }, p1 = vec1[2];

    HCALL(CONFIG(.function_id = hotcall_ecall_container_of), PTR(&ptr), VAR(b_offset, 'd'));
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), PTR(&ptr, 'd', .dereference = true, .member_offset = offsetof(struct A, z)));

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(((struct A *) ptr)->z, 1);

    ASSERT_EQ(a.z, 1);
    ASSERT_EQ(a.b.y, 0);
    ASSERT_EQ(a.b.x, 0);
}
