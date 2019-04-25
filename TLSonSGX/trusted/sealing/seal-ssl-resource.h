#ifndef _ENCLAVE_SEAL_SSL_H
#define _ENCLAVE_SEAL_SSL_H

#include <stdbool.h>
#include "mbedtls/certs.h"
#include "mbedtls/x509.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "enclave_t.h"
#include "sgx_trts.h"
#include "enclave-utils.h"

mbedtls_x509_crt * unseal_certificate(uint8_t *sealed_cert, uint32_t sealed_size);
void unseal_key(uint8_t *sealed_key, uint32_t sealed_size, mbedtls_pk_context *pk_ctx);
sgx_status_t seal_ssl_resource(uint8_t *unsealed_data, size_t unsealed_data_len, SSL_resource *r);
bool all_ssl_resources_exist(SSL_resource *rs);

#endif
