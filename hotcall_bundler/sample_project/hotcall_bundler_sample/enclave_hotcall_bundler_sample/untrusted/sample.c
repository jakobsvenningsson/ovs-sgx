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

#include "ovs-benchmark.h"

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

int cmpfunc (const void * a, const void * b) {
   return ( *(unsigned int*)a - *(unsigned int*)b );
}

#define ROUNDS 10000
#define WARM_UP ROUNDS/10
#define ITERATIONS 10

unsigned int
 benchmark_bundle_if() {
    unsigned int rounds_1[ROUNDS];
    unsigned int rounds_2[ROUNDS];
    unsigned int rounds_3[ROUNDS];

    bool ret;
    for(int i = 0; i < (ROUNDS + WARM_UP); ++i) {
        //long unsigned xs[32 * 1024];
        //for(int j = 0; j < 32 * 1024; ++j) {
        //    xs[j] = rand();
        //}
        BEGIN
        hotcall_bundle_begin(&sm_ctx, NULL);
        HCALL(&sm_ctx, ecall_always_true, false, &ret, 0, NULL);
        char fmt[] = "s";
        IF(
            &sm_ctx,
            1,
            1,
            1,
            fmt,
            1,
            (unsigned int *) &ret
        );
        THEN(HCALL(&sm_ctx, ecall_foo, false, NULL, 0, NULL));
        hotcall_bundle_end(&sm_ctx);
        CLOSE
        if(i >= WARM_UP) {
            rounds_1[i - WARM_UP] = GET_TIME
        }
    }

    qsort(rounds_1, ROUNDS, sizeof(unsigned int), cmpfunc);
    return rounds_1[ROUNDS / 2];
}

unsigned int
benchmark_vanilla_if() {
    unsigned int rounds_1[ROUNDS];
    bool ret;
    for(int i = 0; i < (ROUNDS + WARM_UP); ++i) {
        //long unsigned xs[32 * 1024];
        //for(int j = 0; j < 32 * 1024; ++j) {
        //    xs[j] = rand();
       // }
        BEGIN
        HCALL(&sm_ctx, ecall_always_true, false, &ret, 0, NULL);
        if(ret) {
            HCALL(&sm_ctx, ecall_foo, false, NULL, 0, NULL)
        }
        CLOSE
        if(i < WARM_UP) {
            continue;
        }
        rounds_1[i - WARM_UP] = GET_TIME;
    }
    qsort(rounds_1, ROUNDS, sizeof(unsigned int), cmpfunc);
    return rounds_1[ROUNDS / 2];
}


unsigned int
benchmark_bundle_for_naive() {
    unsigned int rounds[ROUNDS];
    bool ret;
    for(int i = 0; i < (ROUNDS + WARM_UP); ++i) {
        //long unsigned xs[32 * 1024];
        //for(int j = 0; j < 32 * 1024; ++j) {
        //    xs[j] = rand();
       // }
        BEGIN

        HCALL(&sm_ctx, ecall_always_true, false, &ret, 0, NULL);
        if(ret) {
            HCALL(&sm_ctx, ecall_foo, false, NULL, 0, NULL)
        }
        CLOSE
        if(i < WARM_UP) {
            continue;
        }
        rounds_1[i - WARM_UP] = GET_TIME;
    }
    qsort(rounds_1, ROUNDS, sizeof(unsigned int), cmpfunc);
    return rounds_1[ROUNDS / 2];
}

/* Application entry */
int SGX_CDECL main(int argc, char *argv[])
{
  /* Initialize the enclave */
  if(initialize_enclave() < 0){
   return -1;
  }

  ecall_configure_hotcall(global_eid);

  hotcall_init(&sm_ctx, global_eid);

  unsigned int (*benchmark_function_if)();
  printf("%s\n", argv[1]);
  if(strcmp(argv[1], "vanilla") == 0) {
      benchmark_function_if = benchmark_vanilla_if;
  } else if(strcmp(argv[1], "bundle") == 0) {
      benchmark_function_if = benchmark_bundle_if;
  }

  unsigned int avg_median1 = 0;
  for(int n = 0; n < ITERATIONS; ++n) {
      printf("Running iteration %d.\n", n);
      unsigned int r1;
      r1 = benchmark_function_if();
      avg_median += r1;
  }
  printf("Average median: %u.\n", avg_median / ITERATIONS);
 /* bool ret;
  hotcall_bundle_begin(&sm_ctx, NULL);

  HCALL(&sm_ctx, ecall_always_true, false, &ret, 0, NULL);
  char fmt[] = "s";
  IF(
      &sm_ctx,
      0,
      1,
      0,
      fmt,
      1,
      (unsigned int *) &ret
  );
  THEN(HCALL(&sm_ctx, ecall_foo, false, NULL, 0, NULL));

  hotcall_bundle_end(&sm_ctx);*/


  hotcall_destroy(&sm_ctx);

  return 0;
}
