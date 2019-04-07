#include "enclave-utils.h"
#include <stdarg.h>
#include <stdio.h>      /* vsnprintf */

/*
 * printf:
 *   Invokes OCALL to display the enclave buffer to the terminal.
 */
void printf(const char *fmt, ...)
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
