#ifndef _H_TRUSTED_HOTCALL_EXECUTE_FUNCTION_
#define _H_TRUSTED_HOTCALL_EXECUTE_FUNCTION_

#include "hotcall_config.h"
#include <stdint.h>

static inline void
execute_function(struct hotcall_config *hotcall_config, uint8_t function_id, int n_iters, int n_params, void *args[n_iters][n_params]) {
    if(hotcall_config->batch_execute_function) {
        hotcall_config->batch_execute_function(function_id, n_iters, n_params, args);
    } else {
        for(int i = 0; i < n_iters; ++i) {
            hotcall_config->execute_function(function_id, args[i], NULL);
        }
    }
}

#endif