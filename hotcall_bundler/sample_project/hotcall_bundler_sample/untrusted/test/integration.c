#include "gtest/gtest.h"
#include "test.h"
#include "hotcall-untrusted.h"
#include "functions.h"

#include "hotcall_map.h"

TEST(integration,1) {
    //Contract:
    hotcall_test_setup();

    int buffer[15], map_out[15];
    int buf_size = 15;
    unsigned int n = 0;

    BUNDLE_BEGIN();

    HCALL(
        CONFIG(.function_id = hotcall_ecall_read_buffer, .has_return = true ),
        VAR(buffer, 'd'), VAR(buf_size, 'd'), VAR(n, 'd')
    );
    MAP(

        ((struct map_config) { .function_id = hotcall_ecall_plus_one_ret }),
        VECTOR(buffer, 'd', &n),
        VECTOR(map_out, 'd', &n)
    );

    BUNDLE_END();

    hotcall_test_teardown();

    ASSERT_EQ(n, 10);
    for(int i = 0; i < n; ++i) {
        ASSERT_EQ(buffer[i], i);
        ASSERT_EQ(map_out[i], i + 1);
    }
}
/*
TEST(integration,2) {
    hotcall_test_setup();
    unsigned int n_iters = 10;
    int y = 5, counter = 0;
    int *x = NULL;
    int xs[n_iters] = { 1,1,1,1,1,1,1,1,1,1 };

    BUNDLE_BEGIN();

    BEGIN_FOR({ .n_iters = &n_iters });
    ASSIGN_PTR(PTR(x, 'd'), VECTOR(xs, 'd'));

        BEGIN_WHILE(
            ((struct while_config) { .predicate_fmt = "d<d" }),
            PTR(x, 'd'), VAR(y, 'd')
        );
            HCALL(CONFIG(.function_id = hotcall_ecall_plus_one), PTR(x, 'd'));
        END_WHILE();

    END_FOR();

    BUNDLE_END();

    hotcall_test_teardown();

    for(int i = 0; i < n_iters; ++i) {
        ASSERT_EQ(xs[i], 5);
    }
}*/
