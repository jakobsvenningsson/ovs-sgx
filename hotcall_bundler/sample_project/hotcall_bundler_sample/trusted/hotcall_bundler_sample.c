#include <stdarg.h>
#include <stdio.h>      /* vsnprintf */

#include "hotcall_bundler_sample.h"
#include "hotcall_bundler_sample_t.h"  /* print_string */
#include "functions.h"

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
execute_function(struct function_call *fc){

    argument_list * args;
    args       = &fc->args;

    switch (fc->id) {
        case hotcall_ecall_always_true:
            *(bool *) fc->return_value = ecall_always_true();
            break;
        case hotcall_ecall_always_false:
            *(bool *) fc->return_value = ecall_always_false();
            break;
        case hotcall_ecall_foo:
            ecall_foo();
            break;
        case hotcall_ecall_bar:
            ecall_bar();
            break;
        case hotcall_ecall_plus_one:
            ecall_plus_one((int *) args->args[0]);
            break;
        case hotcall_ecall_greater_than_two:
            *(bool *) fc->return_value = ecall_greater_than_two((int *) args->args[0]);
            break;
        case hotcall_ecall_greater_than_y:
            *(bool *) fc->return_value = ecall_greater_than_y((int *) args->args[0], *(int *) args->args[1]);
            break;
        case hotcall_ecall_plus_plus:
            ecall_plus_plus((int *) args->args[0], (int *) args->args[1]);
            break;
        case hotcall_ecall_plus_one_ret:
            *(int *) fc->return_value = ecall_plus_one_ret(*(int *) args->args[0]);
            break;
        case hotcall_ecall_plus:
            *(int *) fc->return_value = ecall_plus(*(int *) args->args[0], *(int *) args->args[1]);
            break;
        default:
            printf("unknown hotcall function %d.\n", fc->id);
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
ecall_foo() {
  //printf("Calling foo!\n");
}

void
ecall_bar() {
  //printf("Calling bar!\n");
}

void
ecall_plus_one(int *x) {
    printf("ecall_plus_one %d \n", *x);
    ++*x;
}

int
ecall_plus_one_ret(int x) {
    return ++x;
}

bool
ecall_greater_than_two(int *x) {
    printf("%d \n", *x);
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
