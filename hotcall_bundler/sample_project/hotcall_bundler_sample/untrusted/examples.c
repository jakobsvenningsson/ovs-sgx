#include "examples.h"
#include "hotcall-untrusted.h"
#include "functions.h"

void hotcall_bundle_example_if(struct shared_memory_ctx *sm_ctx) {
    int x = 0;
    hotcall_bundle_begin(sm_ctx, NULL);
    bool res1, res2, res3;
    HCALL(sm_ctx, ecall_always_false, false, &res1, 0, NULL);
    HCALL(sm_ctx, ecall_always_false, false, &res2, 0, NULL);
    HCALL(sm_ctx, ecall_always_true, false, &res3, 0, NULL);
    int n_variables = 3;
    char fmt[] = "(b&b)&b";
    struct predicate_variable variables[n_variables] = {
        (struct predicate_variable) { &res1, VARIABLE_TYPE, 'b' },
        (struct predicate_variable) { &res2, VARIABLE_TYPE, 'b' },
        (struct predicate_variable) { &res3, VARIABLE_TYPE, 'b' },
    };
    struct predicate predicate = {
        .expected = 1,
        .fmt = fmt,
        .n_variables = n_variables,
        .variables = variables
    };
    struct if_args if_args = {
        .then_branch_len = 1,
        .else_branch_len = 2,
        .predicate = predicate,
        .return_if_false = false
    };
    IF(
        sm_ctx,
        &if_args
    );
    THEN(
        void *args[1] = { &x };
        HCALL(sm_ctx, ecall_plus_one, false, NULL, 1, args)
    );
    ELSE(
        HCALL(sm_ctx, ecall_plus_one, false, NULL, 1, args);
        HCALL(sm_ctx, ecall_plus_one, false, NULL, 1, args);
    );
    HCALL(sm_ctx, ecall_plus_one, false, NULL, 1, args);
    hotcall_bundle_end(sm_ctx);

    printf("x: %d\n", x);
}

void hotcall_bundle_example_for(struct shared_memory_ctx *sm_ctx) {
    hotcall_bundle_begin(sm_ctx, NULL);
    unsigned int n_ecalls = 3, n_params = 2;
    int xs[N_FOR_ITERS] = { 0 };
    int ys[N_FOR_ITERS] = { 0 };
    void *args[n_params] = { xs, ys };

    struct for_args for_args = {
        .n_iters = N_FOR_ITERS,
        .n_rows = n_ecalls
    };
    BEGIN_FOR(sm_ctx, &for_args);
    HCALL(sm_ctx, ecall_plus_plus, false, NULL, 2, args);
    HCALL(sm_ctx, ecall_plus_one, false, NULL, 1, args);
    HCALL(sm_ctx, ecall_plus_one, false, NULL, 1, args);
    END_FOR(sm_ctx, &for_args);
    hotcall_bundle_end(sm_ctx);

    for(int i = 0; i < N_FOR_ITERS; ++i) {
        printf("%d ", xs[i]);
    }
    printf("\n");

    for(int i = 0; i < N_FOR_ITERS; ++i) {
        printf("%d ", ys[i]);
    }
    printf("\n");
}

void hotcall_bundle_example_while(struct shared_memory_ctx *sm_ctx) {
    hotcall_bundle_begin(sm_ctx, NULL);
    unsigned int n_ecalls = 2, n_params = 1, n_variables = 3;
    char fmt[] = "b&b|b";
    int x = 0;
    bool b = false;
    void *args[n_params] = { &x };
    struct function_call fc = {
        .id = hotcall_ecall_greater_than_two,
        .args = (argument_list) {
            .n_args = 1,
            .args = { &x }
        }
    };
    struct predicate_variable variables[n_variables] = {
        (struct predicate_variable) { &fc, FUNCTION_TYPE, 'b' },
        (struct predicate_variable) { &fc, FUNCTION_TYPE, 'b' },
        (struct predicate_variable) { &b, VARIABLE_TYPE, 'b' }
    };
    struct predicate predicate = {
        .expected = 0,
        .fmt = fmt,
        .n_variables = n_variables,
        .variables = variables
    };
    struct while_args while_args = {
        .predicate = predicate,
        .n_rows = n_ecalls
    };
    BEGIN_WHILE(
        sm_ctx,
        &while_args
    );
    HCALL(sm_ctx, ecall_plus_one, false, NULL, n_params, args);
    HCALL(sm_ctx, ecall_plus_one, false, NULL, n_params, args);
    END_WHILE(sm_ctx, &while_args);
    hotcall_bundle_end(sm_ctx);

    printf("X: %d.\n", x);
}

void hotcall_bundle_example_for_each(struct shared_memory_ctx *sm_ctx) {
    hotcall_bundle_begin(sm_ctx, NULL);
    unsigned int n_params = 1, n_iters = 10;
    int xs[n_iters] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    struct function_parameter params_in[n_params] = {
        (struct function_parameter) { .arg = xs, .fmt = 'd', .iter = true }
    };
    struct for_each_args for_each_args = {
        .params_in = (struct function_parameters_in) {
            .params = params_in, .n_params = 1, .iters = n_iters
        },
    };
    FOR_EACH(
      sm_ctx,
      ecall_plus_one,
      &for_each_args
  );
  hotcall_bundle_end(sm_ctx);

  printf("Output: ");
  for(int i = 0; i < n_iters; ++i) {
      printf("%d ", xs[i]);
  }
  printf("\n");
}

void hotcall_bundle_example_filter(struct shared_memory_ctx *sm_ctx) {
    hotcall_bundle_begin(sm_ctx, NULL);
    char fmt[] = "b";
    unsigned int n_params = 1, n_iters = 10, out_length;
    int xs[n_iters] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    int ys[n_iters] = { 0 };

    struct function_call fc = {
        .id = hotcall_ecall_greater_than_two,
        .args = (argument_list) {
            .n_args = 1
        }
    };

    unsigned int n_variables = 1;
    struct predicate_variable variables[n_variables] = {
        (struct predicate_variable) { &fc, FUNCTION_TYPE, 'b' }
    };

    struct function_parameter params_in[n_params] = {
        (struct function_parameter) { .arg = xs, .fmt = 'd', .iter = true }
    };
    struct function_parameter params_out[n_params] = {
        (struct function_parameter) { .arg = ys, .fmt = 'd', .iter = true }
    };

    struct filter_args filter_args = {
        .params_in = (struct function_parameters_in) {
            .params = params_in, .n_params = 1, .iters = n_iters
        },
        .params_out = (struct function_filter_out) {
            .params = params_out, .len = &out_length
        },
        .predicate = (struct predicate)  {
            .expected = 1,
            .fmt = fmt,
            .n_variables = n_variables,
            .variables = variables
        }

    };

    FILTER(
        sm_ctx,
        ecall_greater_than_two,
        &filter_args
    );

  hotcall_bundle_end(sm_ctx);

  printf("Output: ");
  for(int i = 0; i < out_length; ++i) {
      printf("%d ", ys[i]);
  }
  printf("\n");
}

void hotcall_bundle_example_map(struct shared_memory_ctx *sm_ctx) {
    hotcall_bundle_begin(sm_ctx, NULL);

    unsigned int n_params = 1, n_iters = 10;
    int xs[n_iters] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
    int ys[n_iters] = { 0 };

    struct function_parameter params_in[n_params] = {
        (struct function_parameter) { .arg = xs, .fmt = 'd', .iter = true }
    };

    struct map_args map_args = {
        .params_in = (struct function_parameters_in) {
            .params = params_in, .n_params = n_params, .iters = n_iters
        },
        .params_out = (struct function_map_out) {
            .params = ys, .fmt = 'd'
        }
    };

    MAP(
        sm_ctx,
        ecall_plus_one_ret,
        &map_args
    );

    hotcall_bundle_end(sm_ctx);

    printf("Output: ");
    for(int i = 0; i < n_iters; ++i) {
        printf("%d ", ys[i]);
    }
    printf("\n");
}

void hotcall_bundle_example_do_while(struct shared_memory_ctx *sm_ctx) {

}
