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
    struct if_args if_args = {
        .expected = 1,
        .then_branch_len = 1,
        .else_branch_len = 2,
        .fmt = fmt,
        .n_variables = n_variables,
        .variables = variables,
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
    BEGIN_FOR(sm_ctx, N_FOR_ITERS, n_ecalls);
    HCALL(sm_ctx, ecall_plus_plus, false, NULL, 2, args);
    HCALL(sm_ctx, ecall_plus_one, false, NULL, 1, args);
    HCALL(sm_ctx, ecall_plus_one, false, NULL, 1, args);
    END_FOR(sm_ctx, n_ecalls);
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
