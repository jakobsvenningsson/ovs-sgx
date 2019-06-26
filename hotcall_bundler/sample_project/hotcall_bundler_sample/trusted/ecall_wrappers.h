#ifndef _H_ECALL_WRAPPERS_
#define _H_ECALL_WRAPPERS_

#include "functions.h"
#include "ecalls.h"

static void
wrapper_ecall_greater_than_y(void *args[], void *return_value) {
    *(bool *) return_value = ecall_greater_than_y((int *) args[0], *(int *) args[1]);
}

static void
wrapper_ecall_plus_one(void *args[], void *return_value) {
    ecall_plus_one((int *) args[0]);
}

static void
wrapper_ecall_always_true(void *args[], void *return_value) {
    *(bool *) return_value = ecall_always_true();
}

static void
wrapper_ecall_always_false(void *args[], void *return_value) {
    *(bool *) return_value = ecall_always_false();
}

static void
wrapper_ecall_foo(void *args[], void *return_value) {
    ecall_foo();
}

static void
wrapper_ecall_bar(void *args[], void *return_value) {
    ecall_bar();
}

static void
wrapper_ecall_greater_than_two(void *args[], void *return_value) {
    *(bool *) return_value = ecall_greater_than_two((int *) args[0]);
}

static void
wrapper_ecall_plus_plus(void *args[], void *return_value) {
    ecall_plus_plus((int *) args[0], (int *) args[1]);
}

static void
wrapper_ecall_plus_one_ret(void *args[], void *return_value) {
    *(int *) return_value = ecall_plus_one_ret(*(int *) args[0]);
}

static void
wrapper_ecall_plus(void *args[], void *return_value) {
    *(int *) return_value = ecall_plus(*(int *) args[0], *(int *) args[1]);
}

static void
wrapper_ecall_revert(void *args[], void *return_value) {
    *(bool *) return_value = ecall_revert(*(bool *) args[0]);
}


static void
wrapper_ecall_plus_y(void *args[], void *return_value) {
    ecall_plus_y((int *) args[0], *(int *) args[1]);
}

static void
wrapper_ecall_plus_y_v2(void *args[], void *return_value) {
    ecall_plus_y_v2(*(int *) args[0], (int *) args[1]);
}

static void
wrapper_ecall_zero(void *args[], void *return_value) {
    ecall_zero((int *) args[0]);
}

static void
wrapper_ecall_read_buffer(void *args[], void *return_value) {
    *(int *) return_value = ecall_read_buffer((int *) args[0], *(int *) args[1]);
}

static void
wrapper_ecall_count(void *args[], void *return_value) {
    *(int *) return_value = ecall_count();
}

static void
wrapper_ecall_change_ptr_to_ptr(void *args[], void *return_value) {
    ecall_change_ptr_to_ptr((int **) args[0], (int *) args[1]);
}

static void
wrapper_ecall_container_of(void *args[], void *return_value) {
    ecall_offset_of(args[0], -(*(int *) args[1]));
}

static void
wrapper_ecall_offset_of(void *args[], void *return_value) {
    ecall_offset_of(args[0], *(int *) args[1]);
}

static void
wrapper_ecall_offset_of_ret(void *args[], void *return_value) {
    *(void **) return_value = ecall_offset_of_ret(args[0], *(unsigned int *) args[1]);
}

static void
wrapper_ecall_container_of_ret(void *args[], void *return_value) {
    *(void **) return_value = ecall_offset_of_ret(args[0], -(*(unsigned int *) args[1]));
}

static void
wrapper_ecall_strlen(void *args[], void *return_value) {
    *(int *) return_value = ecall_strlen(args[0]);
}

#endif
