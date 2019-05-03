#include "call-table.h"
#include "enclave_t.h"
#include "enclave-utils.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/x509.h"
#include "common.h"

void
execute_function(int function, argument_list * args, void *ret){
    switch (function) {
        case hotcall_ecall_ssl_write:
            *(int *) ret = ecall_ssl_write((char *) args->args[0], *(int *) args->args[1]);
            break;
        case hotcall_ecall_ssl_read:
            *(int *) ret = ecall_ssl_read((char *) args->args[0], *(int *) args->args[1]);
            break;
        case hotcall_ecall_ssl_library_init:
            ecall_ssl_library_init((char *) args->args[0], *(size_t *) args->args[1], (SSL_resource *) args->args[2], (bool *) args->args[3]);
            break;
        case hotcall_ecall_ssl_ctx_new:
            ecall_ssl_ctx_new();
            break;
        case hotcall_ecall_ssl_new:
            ecall_ssl_new();
            break;
        case hotcall_ecall_ssl_ctx_free:
            ecall_ssl_ctx_free();
            break;
        case hotcall_ecall_ssl_ctx_set_verify:
            ecall_ssl_ctx_set_verify();
            break;
        case hotcall_ecall_ssl_get_peer_certificate:
            ecall_ssl_get_peer_certificate((mbedtls_x509_crt *) args->args[0]);
            break;
        case hotcall_ecall_ssl_load_error_strings:
            ecall_ssl_load_error_strings();
            break;
        case hotcall_ecall_ssl_free:
            ecall_ssl_free();
            break;
        case hotcall_ecall_ssl_get_error:
            *(int *) ret = ecall_ssl_get_error(*(int *) args->args[0]);
            break;
        case hotcall_ecall_ssl_set_fd:
            *(int *) ret = ecall_ssl_set_fd(*(int *) args->args[0]);
            break;
        case hotcall_ecall_ssl_connect:
            *(int *) ret = ecall_ssl_connect();
            break;
        case hotcall_ecall_ssl_accept:
            *(int *) ret = ecall_ssl_accept();
            break;
        case hotcall_ecall_ssl_get_state:
            *(int *) ret = ecall_ssl_get_state();
            break;
        case hotcall_ecall_ssl_shutdown:
            *(int *) ret = ecall_ssl_shutdown();
        default:
            printf("Error, no matching switch case for %d.\n", function);
    }
} /* execute_function */
