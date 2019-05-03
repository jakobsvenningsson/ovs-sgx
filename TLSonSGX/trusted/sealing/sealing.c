#include "enclave_t.h"
#include "sgx_trts.h"
#include "sgx_tseal.h"
#include "sgx_attributes.h"

sgx_status_t ecall_seal(uint8_t* text, size_t text_len, sgx_sealed_data_t *sealed, size_t sealed_len) {
    return sgx_seal_data(0, NULL, text_len, text, sealed_len, sealed);
}

sgx_status_t ecall_unseal(sgx_sealed_data_t *sealed, size_t sealed_len, uint8_t *text, uint32_t text_len) {
    return sgx_unseal_data(sealed, NULL, NULL, text, &text_len);
}
