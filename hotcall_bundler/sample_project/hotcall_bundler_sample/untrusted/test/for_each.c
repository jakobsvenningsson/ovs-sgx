#include "gtest/gtest.h"
#include "test.h"
#include "hotcall-untrusted.h"
#include "functions.h"


TEST(for_each,1) {
    //Contract: for each should apply the hcall on each element in the list
    hotcall_test_setup();
    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();


    hotcall_test_teardown();
    for(int i = 0; i < n_iters; ++i) {
        ASSERT_EQ(xs[i], 3);
        ASSERT_EQ(ys[i], i + 2);
    }
}
