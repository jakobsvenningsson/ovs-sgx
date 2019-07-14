#ifndef _H_ECALL_WRAPPERS_
#define _H_ECALL_WRAPPERS_

#include "functions.h"
#include "ecalls.h"

static void
wrapper_ecall_add_and_count(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        *(int *) args[i][n_params - 1] = ecall_add_and_count(*(int *) args[i][0], *(int *) args[i][1], (int *) args[i][2]);
    }
}

static void
wrapper_ecall_get_addr(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        args[i][n_params - 1] = ecall_get_addr(args[i][0]);
    }
}

static void
wrapper_ecall_greater_than_y(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        *(bool *) args[i][n_params - 1] = ecall_greater_than_y((int *) args[i][0], *(int *) args[i][1]);
    }
}

static void
wrapper_ecall_plus_one(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_plus_one((int *) args[i][0]);
    }
}

static void
wrapper_ecall_always_true(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        *(bool *) args[i][n_params - 1] = ecall_always_true();
    }
}

static void
wrapper_ecall_always_false(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        *(bool *) args[i][n_params - 1] = ecall_always_false();
    }
}

static void
wrapper_ecall_foo(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_foo();
    }
}

static void
wrapper_ecall_bar(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_bar();
    }
}

static void
wrapper_ecall_greater_than_two(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        *(bool *) args[i][n_params - 1] = ecall_greater_than_two((int *) args[i][0]);
    }
}

static void
wrapper_ecall_plus_plus(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_plus_plus((int *) args[i][0], (int *) args[i][1]);
    }
}

static void
wrapper_ecall_plus_one_ret(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        *(int *) args[i][n_params - 1] = ecall_plus_one_ret(*(int *) args[i][0]);
    }
}

static void
wrapper_ecall_plus(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        *(int *) args[i][n_params - 1] = ecall_plus(*(int *) args[i][0], *(int *) args[i][1]);
    }
}

static void
wrapper_ecall_revert(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        *(bool *) args[i][n_params - 1] = ecall_revert(*(bool *) args[i][0]);
    }
}


static void
wrapper_ecall_plus_y(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_plus_y((int *) args[i][0], *(int *) args[i][1]);
    }
}

static void
wrapper_ecall_plus_y_v2(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_plus_y_v2(*(int *) args[i][0], (int *) args[i][1]);
    }
}

static void
wrapper_ecall_zero(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_zero((int *) args[i][0]);
    }
}

static void
wrapper_ecall_read_buffer(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        *(int *) args[i][n_params - 1] = ecall_read_buffer((int *) args[i][0], *(int *) args[i][1]);
    }
}

static void
wrapper_ecall_count(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        *(int *) args[i][n_params - 1] = ecall_count();
    }
}

static void
wrapper_ecall_change_ptr_to_ptr(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_change_ptr_to_ptr((int **) args[i][0], (int *) args[i][1]);
    }
}

static void
wrapper_ecall_container_of(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_offset_of(args[i][0], -(*(int *) args[i][1]));
    }
}

static void
wrapper_ecall_offset_of(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_offset_of(args[i][0], *(int *) args[i][1]);
    }
}

static void
wrapper_ecall_offset_of_ret(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        *(void **) args[i][n_params - 1] = ecall_offset_of_ret(args[0], *(unsigned int *) args[i][1]);
    }
}

static void
wrapper_ecall_container_of_ret(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        *(void **) args[i][n_params - 1] = ecall_offset_of_ret(args[0], -(*(unsigned int *) args[i][1]));
    }
}

static void
wrapper_ecall_strlen(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        *(int *) args[i][n_params - 1] = ecall_strlen(args[i][0]);
    }
}


static void
wrapper_ecall_for_each_10_test(unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    for(int i = 0; i < n_iters; ++i) {
        ecall_for_each_10_test(*(uint8_t *) args[i][0], *(uint8_t *) args[i][1], (int *) args[i][2], *(unsigned int *) args[i][3], *(unsigned int *) args[i][4]);
    }
}



#endif
