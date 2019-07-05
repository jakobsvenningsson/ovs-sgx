#include "map.h"
#include "predicate.h"
#include "execute_function.h"
#include "parameter.h"

void
hotcall_handle_map(struct ecall_queue_item *qi, const struct hotcall_config *hotcall_config, struct queue_context *queue_ctx, struct batch_status * batch_status) {
    struct hotcall_map *ma = &qi->call.ma;

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
    parse_arguments(ma->params, n_iters, n_params, args, 0);
    //execute_function(hotcall_config, ma->config->function_id, n_iters, n_params, args);
    hotcall_config->execute_function(ma->config->function_id, n_iters, n_params, args);

}
