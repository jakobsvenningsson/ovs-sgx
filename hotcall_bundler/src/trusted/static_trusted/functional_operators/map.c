#include "map.h"
#include "boolean_expression_translator.h"
#include "execute_function.h"

void
hotcall_handle_map(struct hotcall_map *ma, struct hotcall_config *hotcall_config) {
    const unsigned int n_params = ma->config->n_params, n_iters = *ma->config->n_iters;

    #ifdef SGX_DEBUG
    const struct parameter *params_in = NULL;
    for(int i = 0; i < n_params - 1; ++i) {
        if(ma->params[i].type == VECTOR_TYPE || ma->params[i].type == VECTOR_TYPE_v2) {
            params_in = &ma->params[i];
            break;
        }
    }
    sgx_assert(params_in != NULL, "Map input parameters contains no vector.");
    #endif

    void *args[n_iters][n_params];
    parse_arguments(ma->params, n_params, n_iters, args, 0);
    execute_function(hotcall_config, ma->config->function_id, n_iters, n_params, args);
}
