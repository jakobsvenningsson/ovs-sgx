#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include "sgx_eid.h"     /* sgx_enclave_id_t */
#include "sgx-utils.h"
#include "sgx_urts.h"
#include "sgx_uae_service.h"

# define TOKEN_FILENAME   "/home/jakob/test_ovs/ovs/enclave.token"
# define ENCLAVE_FILENAME "/home/jakob/test_ovs/ovs/enclave.signed.so"
//"/home/jakob/test_ovs/ovs/enclave.signed.so"
# define MAX_PATH FILENAME_MAX


/* Check error conditions for loading enclave */
void
print_error_message(sgx_status_t ret) {
    size_t idx = 0;
    size_t ttl = sizeof sgx_errlist/sizeof sgx_errlist[0];

    for (idx = 0; idx < ttl; idx++) {
        if(ret == sgx_errlist[idx].err) {
            if(NULL != sgx_errlist[idx].sug)
                printf("Info: %s\n", sgx_errlist[idx].sug);
            printf("Error: %s\n", sgx_errlist[idx].msg);
            break;
        }
    }

    if (idx == ttl)
        printf("Error code is 0x%X. Please refer to the \"Intel SGX SDK Developer Reference\" for more details.\n", ret);
}

int
initialize_enclave(sgx_enclave_id_t * global_eid) {
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
    /* Call sgx_create_enclave to initialize an enclave instance */
    /* Debug Support: set 2nd parameter to 1 */
    ret = sgx_create_enclave(ENCLAVE_FILENAME, SGX_DEBUG_FLAG, NULL, NULL, global_eid, NULL);
    if (ret != SGX_SUCCESS) {
        print_error_message(ret);
        return -1;
    }
    return 0;
}

void SGX_status(sgx_status_t ret) {
	if (ret!=SGX_SUCCESS){
		abort();
	}
}

void SGX_return_check (int return_value){
    if (return_value == 0) {
      printf("INFO: Application ran with success\n");
    }
    else
    {
        printf("INFO: Application failed\n");
    }
}
