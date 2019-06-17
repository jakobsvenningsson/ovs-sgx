#include "test.h"
#include "gtest/gtest.h"
#include "hotcall_bundler_sample_u.h"
#include "hotcall-bundler-untrusted.h"

struct shared_memory_ctx *test_sm_ctx;
extern sgx_enclave_id_t global_eid;

int
hotcall_run_tests(struct shared_memory_ctx *sm_ctx) {
    test_sm_ctx = sm_ctx;
    return RUN_ALL_TESTS();
}

void
hotcall_test_setup() {
    ecall_configure_hotcall(global_eid);
    hotcall_init(test_sm_ctx, global_eid);
};

void
hotcall_test_teardown() {
    hotcall_destroy(test_sm_ctx);
};

struct shared_memory_ctx *hotcall_test_get_context() {
    return test_sm_ctx;
}
