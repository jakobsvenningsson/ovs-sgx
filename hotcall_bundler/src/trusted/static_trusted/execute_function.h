#ifndef _H_TRUSTED_HOTCALL_EXECUTE_FUNCTION_
#define _H_TRUSTED_HOTCALL_EXECUTE_FUNCTION_

#include "hotcall_config.h"
#include <stdint.h>

static inline void
execute_function(const struct hotcall_config *hotcall_config, uint8_t function_id, int n_iters, int n_params, void *args[n_params][n_iters]) {
    if(hotcall_config->execute_function_legacy) {
        /*for(int i = 0; i < n_iters; ++i) {
            hotcall_config->execute_function_legacy(function_id, (void **) args[i], args[n_params - 1][i]);
        }*/
    } else {
        hotcall_config->execute_function(function_id, n_iters, n_params, args);
    }
}

#endif
