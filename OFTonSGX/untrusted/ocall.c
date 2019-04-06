#include "ocall.h"
#include <stdio.h>      /* vsnprintf */
#include <unistd.h>



void ocall_print(const char *str) {
    printf("%s", str);
}
