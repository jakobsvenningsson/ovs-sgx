#include "/home/jakob/ovs-sgx/hotcall_bundler/lib/list.h"
#include "hotcall-untrusted.h"

static int *transaction_err;
static struct shared_memory_ctx sm_ctx;

extern void make_hotcall(struct hotcall *);

#ifdef BATCHING
#define ASYNC(X) X || is_transaction_in_progress(&sm_ctx.hcall)
#else
#define ASYNC(X) false || is_transaction_in_progress(&sm_ctx.hcall)
#endif

void
hotcall_init(struct shared_memory_ctx *sm_ctx) {
    list_init(&sm_ctx->hcall.ecall_queue);
    sm_ctx->hcall.transaction_in_progress = false;
    sm_ctx->hcall.first_call_of_transaction = -1;

    sm_ctx->pfc.len = 20;
    sm_ctx->pfc.idx = 0;
    sm_ctx->pfc.idx_uint8 = 0;
    sm_ctx->pfc.idx_unsigned = 0;
    sm_ctx->pfc.idx_sizet = 0;
}

void
hotcall_flush(struct shared_memory_ctx *sm_ctx) {
    make_hotcall(&sm_ctx->hcall);
}

/*
void
hotcall_bundle_begin(struct shared_memory_ctx *sm_ctx, int *transaction_error) {
    transaction_err = transaction_error;
    sm_ctx->hcall.transaction_in_progress = true;
    sm_ctx->hcall.first_call_of_transaction = -1;
}

void
hotcall_bundle_end(struct shared_memory_ctx *sm_ctx) {
    hotcall_flush(sm_ctx);
    transaction_err = NULL;
    sm_ctx->hcall.transaction_in_progress = false;
    sm_ctx->hcall.first_call_of_transaction = -1;
}

void
hotcall_bundle_assert_false(struct preallocated_function_calls *pfc, int condition, int error_code, uint8_t cleanup_function) {
    *transaction_err = error_code;
    void **args = pfc->args[pfc->idx];
    args[0] = next_int(pfc, condition);
    args[1] = next_uint8(pfc, cleanup_function);
    HCALL(sm_ctx, transaction_assert_false_, true, NULL, 2, args);
}

void
hotcall_bundle_expected_value(struct preallocated_function_calls *pfc, int expected, int error_code, bool has_else) {
    void **args = pfc->args[pfc->idx];
    args[0] = next_int(pfc, expected);
    args[1] = next_int(pfc, error_code);
    args[2] = transaction_err;
    args[3] = next_bool(pfc, has_else);
    HCALL(transaction_guard, true, NULL, 4, args);
}

void
hotcall_bundle_if_(struct preallocated_function_calls *pfc, int expected, int **conditions, char *type, int n_conditions, int if_len, int else_len) {
    void **args = pfc->args[pfc->idx];
    args[0] = next_int(pfc, expected);
    args[1] = (void **) conditions;
    args[2] = next_int(pfc, n_conditions);
    args[3] = next_int(pfc, if_len);
    args[4] = next_int(pfc, else_len);
    args[5] = type;
    HCALL(transaction_if, true, NULL, 6, args);
}

void
hotcall_transaction_if_null_(struct preallocated_function_calls *pfc, void *condition, int if_len, int else_len) {
    void **args = pfc->args[pfc->idx];
    args[0] = condition;
    args[1] = next_int(pfc, if_len);
    args[2] = next_int(pfc, else_len);
    HCALL(transaction_if_null, true, NULL, 3, args);
}
*/
/*
bool
is_transaction_in_progress(struct hotcall *hcall) {
    return hcall->transaction_in_progress;
}

int
first_call_of_transaction(struct hotcall *hcall) {
    return hcall->first_call_of_transaction;
}
*/
