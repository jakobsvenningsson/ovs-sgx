#include "hotcall_bundler_t.h"  /* print_string */

void sgx_assert(bool condition, char *msg) {
    if(!condition) {
        printf("Assertion failed, %s\n", msg);
        //ocall_assert(msg);
    }
}
