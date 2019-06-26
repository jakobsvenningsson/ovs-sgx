#include "for_each.h"
#include "boolean_expression_translator.h"
#include "execute_function.h"

void
hotcall_handle_for_each(struct hotcall_for_each *tor, struct hotcall_config *hotcall_config) {
    unsigned int n_iters = *tor->config->n_iters;
    unsigned int n_params = tor->config->n_params;

    #ifdef SGX_DEBUG
    struct parameter *params_in = NULL;
    for(int i = 0; i < n_params; ++i) {
        if(tor->params[i].type == VECTOR_TYPE || tor->params[i].type == VECTOR_TYPE_v2) {
            params_in = &tor->params[i];
            break;
        }
    }
    sgx_assert(params_in != NULL, "For each input parameters contains no vector. Undefined behaviour!");
    #endif

    void *args[n_iters][n_params];
    parse_arguments(tor->params, n_params, n_iters, args, 0);
    execute_function(hotcall_config, tor->config->function_id, n_iters, n_params, args);
}