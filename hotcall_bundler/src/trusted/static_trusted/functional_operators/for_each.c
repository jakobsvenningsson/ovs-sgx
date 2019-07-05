#include "for_each.h"
#include "predicate.h"
#include "execute_function.h"
#include "parameter.h"

void
hotcall_handle_for_each(struct ecall_queue_item *qi, const struct hotcall_config *hotcall_config, struct queue_context *queue_ctx, struct batch_status * batch_status) {
    struct hotcall_for_each *tor = &qi->call.tor;

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
    parse_arguments(tor->params, n_iters, n_params, args, 0);
    //execute_function(hotcall_config, tor->config->function_id, n_iters, n_params, args);
    hotcall_config->execute_function(tor->config->function_id, n_iters, n_params, args);

}
