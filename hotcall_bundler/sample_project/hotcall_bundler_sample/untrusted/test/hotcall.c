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

    struct parameter vec1[] = { VAR(x_ptr, 'd'), PTR_v2(&vec1[0], .dereference = true) }, p1 = vec1[1];
    struct parameter vec2[] = { VAR(y_ptr, 'd'), PTR_v2(&vec2[0], .dereference = true) }, p2 = vec2[1];

    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one_ret, .has_return = true ), p1, VAR(y, 'd'));
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one_ret, .has_return = true ), p2, VAR(z, 'd'));

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

    struct parameter vec1[] = { VAR(x_ptr, 'd'), PTR_v2(&vec1[0], .dereference = false) }, p1 = vec1[1];
    struct parameter vec2[] = { VAR(y, 'd'), PTR_v2(&vec2[0], .dereference = false) }, p2 = vec2[1];

    HCALL(CONFIG(.function_id = hotcall_ecall_change_ptr_to_ptr), p1, p2);

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(*x_ptr, 1);
    ASSERT_EQ(y, 1);
}

TEST(hotcall, 7) {
    /* Contract:  RETURN should stop execution of the bundle it appears inside of. */

    hotcall_test_setup();


    int x = 0, y = 1;

    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));

    BUNDLE_BEGIN();

    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));

    RETURN;

    BUNDLE_END();

    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));
    IF(((struct if_config) { .predicate_fmt = "d" }), VAR(y, 'd'));
    THEN
        RETURN;
    END

    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));

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

TEST(hotcall, 8) {
    /* Contract:  If, for, and while loops backtracks in the execution queue in order to calculate the length of its body. */

    hotcall_test_setup();

    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();
    sm_ctx->hcall.idx = 0;

    int x = 0, y = 0;

    for(int i = 0; i < MAX_FCS - 1; ++i) {
        HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(x, 'd'));
    }


    BUNDLE_BEGIN();

    IF(((struct if_config) { .predicate_fmt = "!d"}), VAR(y, 'd'));
    THEN
        HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(y, 'd'));
        HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(y, 'd'));
        HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), VAR(y, 'd'));
    END

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(x, 199);
    ASSERT_EQ(y, 3);
}

TEST(hotcall, 9) {
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


    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), p1);
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), p2);
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), p2);

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(a.x, 2);
    ASSERT_EQ(a.y, 1);
}

TEST(hotcall, 10) {
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
    struct parameter vec1[] = { VAR(a_ptr, 'd'), STRUCT(&vec1[0], .member_offset = offsetof(struct B, y)), PTR_v2(&vec1[1], .dereference = true) }, p1 = vec1[2];

    BUNDLE_BEGIN();


    int b_offset = offsetof(struct A, b);

    HCALL(CONFIG(.function_id = hotcall_ecall_offset_of), PTR(&a_ptr), VAR(b_offset, 'u'));
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), p1);

    BUNDLE_END();

    hotcall_test_teardown();


    ASSERT_EQ(((struct B *) a_ptr)->x, 0);
    ASSERT_EQ(((struct B *) a_ptr)->y, 1);

    ASSERT_EQ(a.z, 0);
    ASSERT_EQ(a.b.y, 1);
    ASSERT_EQ(a.b.x, 0);
}

TEST(hotcall, 11) {
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

    struct parameter vec1[] = { VAR(ptr, 'd'), STRUCT(&vec1[0], .member_offset = offsetof(struct A, z)), PTR_v2(&vec1[1], .dereference = true) }, p1 = vec1[2];

    HCALL(CONFIG(.function_id = hotcall_ecall_container_of), PTR(&ptr), VAR(b_offset, 'd'));
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), p1);

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(((struct A *) ptr)->z, 1);

    ASSERT_EQ(a.z, 1);
    ASSERT_EQ(a.b.y, 0);
    ASSERT_EQ(a.b.x, 0);
}

TEST(hotcall, 12) {


    /*hotcall_test_setup();

    struct B {
        int x;
        int y;
    };

    struct A {
        int z;
        struct B b;
    };

    A a1 = { 0 }, a2 = { 0 }, a3 = { 0 };
    void *as[] = { &a1.b, &a2.b, &a3.b };
    unsigned int len = 3;
    struct A *a_ptr;
    a3.b.x = 3;
    printf("%p\n", as[0]);

    BUNDLE_BEGIN();
    int offset = offsetof(struct A, b);
    FOR_EACH(
        ((struct for_each_config) { .function_id =  hotcall_ecall_container_of }),
        VECTOR(as, 'p', &len, .dereference = false), VAR(offset, 'd')
    );

    BUNDLE_END();

    printf("%p\n", as[0]);

    BUNDLE_BEGIN();

    FOR_EACH(
        ((struct for_each_config) { .function_id =  hotcall_ecall_plus_one }),
        VECTOR(as, 'p', &len, .dereference = true, .access_member = offsetof(struct A, z))
    );*/

    //BUNDLE_END();

    /*FOR_EACH(
        ((struct for_each_config) { .function_id =  hotcall_ecall_plus_one }), VECTOR(as, 'p', &len)
    );*/
    //BEGIN_FOR(((struct for_config) {
    //    .n_iters = &len
    //}));

    //ASSIGN_PTR(PTR(&a_ptr, 'p'), VECTOR(as, 'p', &len));
    //HCALL(CONFIG(.function_id = hotcall_ecall_container_of), PTR(&a_ptr, 'p'), VAR(offset, 'd'));
    //HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), PTR(&a_ptr, 'p', .dereference = true, .access_member = offsetof(struct A, z)));

    //END_FOR();

    //BUNDLE_END();

    //hotcall_test_teardown();


    //printf("%p %p %p %p %p\n", a_ptr, as[2], &a3, &a3.b, as);


    //ASSERT_EQ(((struct A *) as[2])->b.x, 3);
    //ASSERT_EQ(((struct A *) as[2])->z, 1);


//    ASSERT_EQ(a_ptr->z, 1);
//    ASSERT_EQ(((struct A *) as[2])->z, 1);


/*
    ASSERT_EQ(((struct A *) as[2])->b.x, 0);
    ASSERT_EQ(((struct A *) as[0])->b.x, 0);
    ASSERT_EQ(((struct A *) as[1])->b.x, 0);*/

}
/*
TEST(hotcall, 12) {


    hotcall_test_setup();

    struct B {
        int x;
        int y;
    };

    struct A {
        int z;
        struct B b;
    };

    A a1 = { 0 }, a2 = { 0 }, a3 = { 0 };
    void *as[] = { &a1.b, &a2.b, &a3.b };
    unsigned int len = 3;
    struct A *a_ptr;

    BUNDLE_BEGIN();
    int offset = offsetof(struct A, b);

    BEGIN_FOR(((struct for_config) {
        .n_iters = &len
    }));

    ASSIGN_PTR(PTR(&a_ptr, 'p'), VECTOR(as, 'p', &len));
    HCALL(CONFIG(.function_id = hotcall_ecall_container_of), PTR(&a_ptr, 'p'), VAR(offset, 'd'));
    HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), PTR(&a_ptr, 'p', .dereference = true, .access_member = offsetof(struct A, z)));

    END_FOR();

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(a_ptr->z, 1);
    ASSERT_EQ(((struct A *) as[i])->z, 1);

    for(int i = 0; i < len; ++i) {
        ASSERT_EQ(((struct A *) as[i])->z, 1);
        ASSERT_EQ(((struct A *) as[i])->b.x, 0);
        ASSERT_EQ(((struct A *) as[i])->b.y, 0);
    }



}*/
