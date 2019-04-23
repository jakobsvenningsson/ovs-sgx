#pragma once

#include "sgx_error.h"
#include "sgx_eid.h"     /* sgx_enclave_id_t */

#define TOKEN_FILENAME   "/home/jakob/ovs-sgx/ovs/tls_enclave.token"
#define ENCLAVE_FILENAME "/home/jakob/ovs-sgx/ovs/tls_enclave.signed.so"


void print_error_message(sgx_status_t ret);
int initialize_enclave(sgx_enclave_id_t *eid);

#if defined(__cplusplus)
extern "C" {
#endif

//extern sgx_enclave_id_t global_eid;    /* global enclave id */

#define MAKE_ECALL_ARGS(FUNC, ecall_ret, global_eid, args...) \
  do { \
    sgx_status_t sgx_ret = ecall_ ## FUNC(global_eid, ecall_ret, args); \
    if( sgx_ret != SGX_SUCCESS) { \
      print_error_message(sgx_ret); \
      printf("ecall failed in ecall_%s\n", #FUNC); \
    } \
  } while(0)

#define MAKE_ECALL(FUNC, ecall_ret, global_eid) \
  do { \
    sgx_status_t sgx_ret = ecall_ ## FUNC(global_eid, ecall_ret); \
    if( sgx_ret != SGX_SUCCESS) { \
      print_error_message(sgx_ret); \
      printf("ecall failed in ecall_%s\n", #FUNC); \
    } \
  } while(0)


typedef struct _sgx_errlist_t {
    sgx_status_t err;
    const char *msg;
    const char *sug; /* Suggestion */
} sgx_errlist_t;

/* Error code returned by sgx_create_enclave */
static sgx_errlist_t sgx_errlist[] = {
    {
        SGX_ERROR_UNEXPECTED,
        "Unexpected error occurred.",
        NULL
    },
    {
        SGX_ERROR_INVALID_PARAMETER,
        "Invalid parameter.",
        NULL
    },
    {
        SGX_ERROR_OUT_OF_MEMORY,
        "Out of memory.",
        NULL
    },
    {
        SGX_ERROR_ENCLAVE_LOST,
        "Power transition occurred.",
        "Please refer to the sample \"PowerTransition\" for details."
    },
    {
        SGX_ERROR_INVALID_ENCLAVE,
        "Invalid enclave image.",
        NULL
    },
    {
        SGX_ERROR_INVALID_ENCLAVE_ID,
        "Invalid enclave identification.",
        NULL
    },
    {
        SGX_ERROR_INVALID_SIGNATURE,
        "Invalid enclave signature.",
        NULL
    },
    {
        SGX_ERROR_OUT_OF_EPC,
        "Out of EPC memory.",
        NULL
    },
    {
        SGX_ERROR_NO_DEVICE,
        "Invalid SGX device.",
        "Please make sure SGX module is enabled in the BIOS, and install SGX driver afterwards."
    },
    {
        SGX_ERROR_MEMORY_MAP_CONFLICT,
        "Memory map conflicted.",
        NULL
    },
    {
        SGX_ERROR_INVALID_METADATA,
        "Invalid enclave metadata.",
        NULL
    },
    {
        SGX_ERROR_DEVICE_BUSY,
        "SGX device was busy.",
        NULL
    },
    {
        SGX_ERROR_INVALID_VERSION,
        "Enclave version was invalid.",
        NULL
    },
    {
        SGX_ERROR_INVALID_ATTRIBUTE,
        "Enclave was not authorized.",
        NULL
    },
    {
        SGX_ERROR_ENCLAVE_FILE_ACCESS,
        "Can't open enclave file.",
        NULL
    },
};

#if defined(__cplusplus)
}
#endif
