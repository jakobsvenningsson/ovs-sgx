#include "seal-ssl-resource.h"

mbedtls_x509_crt *
unseal_certificate(uint8_t *sealed_cert, uint32_t sealed_size) {
    uint32_t unsealed_size = sgx_get_encrypt_txt_len((sgx_sealed_data_t *) sealed_cert);
    uint8_t cert_unsealed[unsealed_size];

    sgx_status_t res;
    res = ecall_unseal((sgx_sealed_data_t *) sealed_cert, sealed_size, cert_unsealed, unsealed_size);
    if(res != SGX_SUCCESS) {
        printf("Failed to unseal certificate. Error code %u.\n", res);
        return NULL;
    }
    mbedtls_x509_crt *crt = malloc(2048);
    mbedtls_x509_crt_init(crt);
    if (mbedtls_x509_crt_parse(crt, cert_unsealed, unsealed_size) != 0) {
        printf("Failed to parse certificate.\n");
        return NULL;
    }

    return crt;
}

void
unseal_key(uint8_t *sealed_key, uint32_t sealed_size, mbedtls_pk_context *pk_ctx) {

    uint32_t unsealed_size = sgx_get_encrypt_txt_len((sgx_sealed_data_t *) sealed_key);
    uint8_t key_unsealed[unsealed_size + 1];
    key_unsealed[unsealed_size] = '\0';

    sgx_status_t res;
    res = ecall_unseal((sgx_sealed_data_t *) sealed_key, sealed_size, key_unsealed, unsealed_size);
    if(res != SGX_SUCCESS) {
        printf("Failed to unseal key. Error code %u.\n", res);
        return;
    }

    int parse_ret;
    if((parse_ret = mbedtls_pk_parse_key(pk_ctx, key_unsealed, unsealed_size + 1, NULL, 0)) != 0) {
        print_mbedtls_error(parse_ret);
    }
}



sgx_status_t
seal_ssl_resource(uint8_t *unsealed_data, size_t unsealed_data_len, SSL_resource *r) {
    uint32_t sealed_data_len = sgx_calc_sealed_data_size(0, unsealed_data_len);
    printf("Seal length %u\n", sealed_data_len);
    uint8_t* sealed_data = (uint8_t*) malloc(sealed_data_len);
    sgx_status_t sgx_res;
    sgx_res = ecall_seal(unsealed_data, unsealed_data_len, (sgx_sealed_data_t *) sealed_data, sealed_data_len);
    if(sgx_res != SGX_SUCCESS) {
        return sgx_res;
    }

    r->len = sealed_data_len;
    r->unsealed_len = unsealed_data_len;
    memcpy(r->sealed_data, sealed_data, r->len);

    free(sealed_data);
    return sgx_res;
}

bool
all_ssl_resources_exist(SSL_resource *rs) {
    for(size_t i = 0; i < N_SSL_RESOURCES; ++i) {
        if(!rs[i].exists) {
            return false;
        }
    }
    return true;
}
