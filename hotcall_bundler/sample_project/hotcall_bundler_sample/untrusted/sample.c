#include <string.h>
#include <assert.h>

#include <unistd.h>
#include <pwd.h>
#include <libgen.h>
#include <stdlib.h>

#include <sgx_urts.h>
#include "sample.h"

#include "hotcall_bundler_sample_u.h"

#include "hotcall-untrusted.h"
#include "functions.h"
#include "examples.h"
#include <gtest/gtest.h>
#include "test/test.h"
#include "benchmark/benchmark.h"


sgx_enclave_id_t global_eid = 0;

static struct shared_memory_ctx sm_ctx;

/* OCall functions */
void ocall_print(const char *str)
{
    printf("%s", str);
}

int initialize_enclave(void)
{
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
    ret = sgx_create_enclave(HOTCALL_BUNDLER_SAMPLE_FILENAME, SGX_DEBUG_FLAG, NULL, NULL, &global_eid, NULL);
    if (ret != SGX_SUCCESS) {
        printf("Failed to initialize enclave.\n");
        return -1;
    }
    return 0;
}

#define ROUNDS 10000
#define ITERATIONS 10

/* Application entry */
int SGX_CDECL main(int argc, char *argv[])
{
    ::testing::InitGoogleTest(&argc, argv);

    /* Initialize the enclave */
    if(initialize_enclave() < 0){
        return -1;
    }

    if(argc > 1 && !strcmp(argv[1], "-t")) {
        printf("Running test...\n");
        return hotcall_run_tests(&sm_ctx);
    }

    ecall_configure_hotcall(global_eid);
    hotcall_init(&sm_ctx, global_eid);

    benchmark(&sm_ctx, benchmark_hotcall, ROUNDS, ITERATIONS);

    hotcall_destroy(&sm_ctx);

  return 0;
}
