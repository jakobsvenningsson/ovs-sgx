#include <stdarg.h>
#include <stdio.h>      /* vsnprintf */

#include "hotcall_bundler_sample.h"
#include "hotcall_bundler_sample_t.h"  /* print_string */
#include "functions.h"
#include "hotcall_config.h"
#include "ecalls.h"
#include <math.h>


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
        case hotcall_ecall_plus_y_v2:
            ecall_plus_y_v2(*(int *) args[0], (int *) args[1]);
            break;
        case hotcall_ecall_zero:
            ecall_zero((int *) args[0]);
            break;
        case hotcall_ecall_read_buffer:
            *(int *) return_value = ecall_read_buffer((int *) args[0], *(int *) args[1]);
            break;
        case hotcall_ecall_count:
            *(int *) return_value = ecall_count();
            break;
        case hotcall_ecall_change_ptr_to_ptr:
            ecall_change_ptr_to_ptr((int **) args[0], (int *) args[1]);
            break;
        case hotcall_ecall_container_of:
            ecall_offset_of(args[0], -(*(int *) args[1]));
            break;
        case hotcall_ecall_offset_of:
            ecall_offset_of(args[0], *(int *) args[1]);
            break;
        case hotcall_ecall_offset_of_ret:
            *(void **) return_value = ecall_offset_of_ret(args[0], *(unsigned int *) args[1]);
            break;
        case hotcall_ecall_container_of_ret:
            *(void **) return_value = ecall_offset_of_ret(args[0], -(*(unsigned int *) args[1]));
            break;
        case hotcall_ecall_strlen:
            *(int *) return_value = ecall_strlen(args[0]);
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

void printf(const char *fmt, ...) {
    char buf[BUFSIZ] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, BUFSIZ, fmt, ap);
    va_end(ap);
    ocall_print(buf);
}
