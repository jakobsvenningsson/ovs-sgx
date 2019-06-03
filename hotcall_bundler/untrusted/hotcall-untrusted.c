#include "hotcall_bundler_u.h"
#include "hotcall-untrusted.h"
#include <pthread.h>

static int *transaction_err;

static sgx_enclave_id_t global_eid;
struct shared_memory_ctx *sm_ctx;

void *
start_enclave_thread(void * vargp){
    printf("start_enclave_thread\n");
    int ecall_return;
    ecall_start_poller(global_eid, &ecall_return, sm_ctx);
    if (ecall_return == 0) {
        printf("Application ran with success\n");
    } else {
        printf("Application failed %d \n", ecall_return);
    }
}

void
hotcall_init(struct shared_memory_ctx *ctx, sgx_enclave_id_t eid) {

    ctx->hcall.transaction_in_progress = false;
    ctx->hcall.first_call_of_transaction = -1;
    ctx->hcall.queue_length = 0;

    ctx->pfc.len = 20;
    ctx->pfc.idx = 0;
    ctx->pfc.idx_uint8 = 0;
    ctx->pfc.idx_unsigned = 0;
    ctx->pfc.idx_sizet = 0;

    global_eid = eid;
    sm_ctx = ctx;

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, start_enclave_thread, NULL);
}

void
hotcall_bundle_flush(struct shared_memory_ctx *sm_ctx) {
    make_hotcall(&sm_ctx->hcall);
}

void
hotcall_destroy(struct shared_memory_ctx *sm_ctx) {
  HCALL_CONTROL(sm_ctx, DESTROY, false, 0, NULL);
}






/*
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

*/
