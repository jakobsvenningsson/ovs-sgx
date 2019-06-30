#include <stdarg.h>
#include <stdio.h>      /* vsnprintf */

#include "hotcall_bundler_sample.h"
#include "hotcall_bundler_sample_t.h"  /* print_string */
#include "functions.h"
#include "hotcall_config.h"
#include "ecall_wrappers.h"

#include <math.h>

#define CALL_TABLE_CAPACITY 256

void *call_table[CALL_TABLE_CAPACITY] = {
    [hotcall_ecall_always_true] = wrapper_ecall_always_true,
    [hotcall_ecall_always_false] = wrapper_ecall_always_false,
    [hotcall_ecall_foo] = wrapper_ecall_foo,
    [hotcall_ecall_bar] = wrapper_ecall_bar,
    [hotcall_ecall_plus_one] = wrapper_ecall_plus_one,
    [hotcall_ecall_greater_than_two] = wrapper_ecall_greater_than_two,
    [hotcall_ecall_plus_one_ret] = wrapper_ecall_plus_one_ret,
    [hotcall_ecall_plus] = wrapper_ecall_plus,
    [hotcall_ecall_plus_y] = wrapper_ecall_plus_y,
    [hotcall_ecall_zero] = wrapper_ecall_zero,
    [hotcall_ecall_plus_y_v2] = wrapper_ecall_plus_y_v2,
    [hotcall_ecall_count] = wrapper_ecall_count,
    [hotcall_ecall_read_buffer] = wrapper_ecall_read_buffer,
    [hotcall_ecall_change_ptr_to_ptr] = wrapper_ecall_change_ptr_to_ptr,
    [hotcall_ecall_container_of] = wrapper_ecall_container_of,
    [hotcall_ecall_offset_of] = wrapper_ecall_offset_of,
    [hotcall_ecall_container_of_ret] = wrapper_ecall_container_of_ret,
    [hotcall_ecall_offset_of_ret] = wrapper_ecall_offset_of_ret,
    [hotcall_ecall_strlen] = wrapper_ecall_strlen,
    [hotcall_ecall_greater_than_y] = wrapper_ecall_greater_than_y,
    [hotcall_ecall_plus_plus] = wrapper_ecall_plus_plus,
    [hotcall_ecall_for_each_10_test] = wrapper_ecall_for_each_10_test,

};

void
execute_function(uint8_t function_id, unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]) {
    void (*f)(unsigned int n_iters, unsigned int n_params, void *[n_iters][n_params]);
    f = call_table[function_id];
    #ifdef SGX_DEBUG
    if(!f) {
        printf("unknown hotcall function %d.\n", function_id);
    }
    #endif
    f(n_iters, n_params, args);
}

void
ecall_configure_hotcall() {
    struct hotcall_config conf = {
        .execute_function_legacy = NULL,
        .execute_function = execute_function,
        .n_spinlock_jobs = 0,
    };
    struct hotcall_config *config = malloc(sizeof(struct hotcall_config));
    memcpy(config, &conf, sizeof(struct hotcall_config));
    hotcall_register_config(config);
}

void printf(const char *fmt, ...) {
    char buf[BUFSIZ] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    ocall_print(buf);
}
