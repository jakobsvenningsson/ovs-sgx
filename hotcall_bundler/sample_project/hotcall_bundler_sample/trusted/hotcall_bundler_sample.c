#include <stdarg.h>
#include <stdio.h>      /* vsnprintf */

#include "hotcall_bundler_sample.h"
#include "hotcall_bundler_sample_t.h"  /* print_string */
#include "functions.h"
#include "hotcall_config.h"


void sgx_assert(bool condition, char *msg) {
    if(!condition) {
        ocall_assert(msg);
    }
}

void printf(const char *fmt, ...)
{
    char buf[BUFSIZ] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    ocall_print(buf);
}

void
execute_function(uint8_t function_id, void *args[], void *return_value){
    switch (function_id) {
        case hotcall_ecall_always_true:
            *(bool *) return_value = ecall_always_true();
            break;
        case hotcall_ecall_always_false:
            *(bool *) return_value = ecall_always_false();
            break;
        case hotcall_ecall_foo:
            ecall_foo();
            break;
        case hotcall_ecall_bar:
            ecall_bar();
            break;
        case hotcall_ecall_plus_one:
            ecall_plus_one((int *) args[0]);
            break;
        case hotcall_ecall_greater_than_two:
            *(bool *) return_value = ecall_greater_than_two((int *) args[0]);
            break;
        case hotcall_ecall_greater_than_y:
            *(bool *) return_value = ecall_greater_than_y((int *) args[0], *(int *) args[1]);
            break;
        case hotcall_ecall_plus_plus:
            ecall_plus_plus((int *) args[0], (int *) args[1]);
            break;
        case hotcall_ecall_plus_one_ret:
            *(int *) return_value = ecall_plus_one_ret(*(int *) args[0]);
            break;
        case hotcall_ecall_plus:
            *(int *) return_value = ecall_plus(*(int *) args[0], *(int *) args[1]);
            break;
        case hotcall_ecall_revert:
            *(bool *) return_value = ecall_revert(*(bool *) args[0]);
            break;
        case hotcall_ecall_plus_y:
            ecall_plus_y((int *) args[0], *(int *) args[1]);
            break;
        case hotcall_ecall_zero:
            ecall_zero((int *) args[0]);
            break;
        default:
            printf("unknown hotcall function %d.\n", function_id);
            break;
        }
  }

void
ecall_configure_hotcall() {
    struct hotcall_config conf = {
        .execute_function = execute_function,
        .n_spinlock_jobs = 0,
    };
    struct hotcall_config *config = malloc(sizeof(struct hotcall_config));
    memcpy(config, &conf, sizeof(struct hotcall_config));
    hotcall_register_config(config);
}

bool
ecall_always_true() {
  return true;
}

bool
ecall_always_false() {
  return false;
}

void
ecall_foo() {}

void
ecall_bar() {}

void
ecall_plus_one(int *x) {
    ++*x;
}

int
ecall_plus_one_ret(int x) {
    return ++x;
}

bool
ecall_greater_than_two(int *x) {
    return *x > 2 ? true : false;
}


bool
ecall_greater_than_y(int *x, int y) {
    return *x > y ? true : false;
}

void
ecall_plus_plus(int *x, int *y) {
    *x = *x + 1;
    *y = *y + 2;
}

int
ecall_plus(int x, int y) {
    return x + y;
}

int
ecall_plus_y(int *x, int y) {
    *x = *x + y;
}

bool
ecall_revert(bool x) {
    return x == true ? false : true;
}

void
ecall_zero(int *x) {
    *x = 0;
}
