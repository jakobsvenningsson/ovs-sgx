#include "enclave_t.h"
#include "sgx_trts.h"
#include "enclave-utils.h"
#include <stdarg.h>
#include <stdio.h>      /* vsnprintf */
#include "mbedtls/error.h"

/*
 * printf:
 *   Invokes OCALL to display the enclave buffer to the terminal.
 */
void
printf(const char *fmt, ...)
{
    char buf[BUFSIZ] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    ocall_print(buf);
}


int sprintf(char* buf, const char *fmt, ...)
{
    va_list ap;
    int ret;
    va_start(ap, fmt);
    ret = vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    return ret;
}

int ENCLAVE_LOG(char* buf, const char *fmt, ...) {
  int ret = 0;
  #ifdef DEBUG_PRINT
  va_list ap;
  va_start(ap, fmt);
  ret = vsnprintf(buf, BUFSIZ, fmt, ap);
  printf("ENCLAVE LOG: %s", buf);
  va_end(ap);
  #endif
  return ret;
}

void print_mbedtls_error(int error_code) {
    char err_buf[256];
    mbedtls_strerror(error_code, err_buf, 256);
    printf("Mbedtls error: %s.\n", err_buf);
}
