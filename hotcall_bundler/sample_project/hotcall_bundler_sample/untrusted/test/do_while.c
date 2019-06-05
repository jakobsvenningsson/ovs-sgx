#include "gtest/gtest.h"
#include "test.h"
#include "hotcall-untrusted.h"
#include "functions.h"

#include "hotcall_do_while.h"


TEST(do_while,1) {
    //Contract: the body of the while loop should execute 3 times and hence x should be 3 after termination.

    hotcall_test_setup();
    struct shared_memory_ctx *sm_ctx = hotcall_test_get_context();

    unsigned int n_params = 1, n_variables = 1, x = 0;

    struct function_parameter params_body[n_params] = {
        (struct function_parameter) { .arg = &x, .fmt = 'u', .iter = false }
    };

    struct hotcall_function fc = {
        .id = hotcall_ecall_greater_than_two,
        .args = (argument_list) {
            .n_args = 1,
            .args = { &x }
        }
    };
    struct predicate_variable variables[n_variables] = {
        (struct predicate_variable) { &fc, FUNCTION_TYPE, 'b' }
    };

    char fmt[] = "b";
    struct do_while_args dw_args = {
        .params = (struct function_parameters_in) {
            .params = params_body, .n_params = 1, .iters = 0,
        },
        .predicate = (struct predicate)  {
            .expected = 0,
            .fmt = fmt,
            .n_variables = n_variables,
            .variables = variables
        }
    };

    DO_WHILE(
        sm_ctx,
        ecall_plus_one,
        &dw_args
    );

    hotcall_bundle_end(sm_ctx);

    hotcall_test_teardown();

    ASSERT_EQ(x, 3);
}
