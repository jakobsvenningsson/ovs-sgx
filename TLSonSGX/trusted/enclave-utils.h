#ifndef _ENCLAVE_UTILS_H
#define _ENCLAVE_UTILS_H

int sprintf(char* buf, const char *fmt, ...);
void printf(const char *fmt, ...);
void print_mbedtls_error(int error_code);

#endif
