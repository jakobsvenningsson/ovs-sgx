#include "filter.h"
#include "predicate.h"

static inline unsigned int
copy_filtered_results(struct vector_parameter *output_vec, const struct vector_parameter *input_vec, unsigned int n_iters, int results[n_iters]) {
    int n_include = 0;
    for(int n = 0; n < n_iters; ++n) {
        if(results[n]) {
            switch(output_vec->fmt) {
                case 'u':
                    ((unsigned int *) output_vec->arg)[n_include] = ((unsigned int *) input_vec->arg)[n];
                    break;
                case 'b':
                    ((bool *) output_vec->arg)[n_include] = ((bool *) input_vec->arg)[n];
                    break;
                case 'd':
                    ((int *) output_vec->arg)[n_include] = ((int *) input_vec->arg)[n];
                    break;
                case 'p':
                    ((void **) output_vec->arg)[n_include] = ((void **) input_vec->arg)[n];
                    break;
                default: SWITCH_DEFAULT_REACHED
            }
            n_include++;
        }
    }
    return n_include;
}

void
hotcall_handle_filter(struct ecall_queue_item *qi, const struct hotcall_config *hotcall_config, struct queue_context *queue_ctx, struct batch_status * batch_status) {
    struct hotcall_filter *fi = &qi->call.fi;
    const unsigned int n_params = fi->config->n_params - 1;
    struct vector_parameter *input_vec = NULL, *output_vec = NULL;
    struct parameter *input, *output;

    // Input vector is specified
    if(fi->config->input_vector) {
        input = fi->config->input_vector;
        input_vec = &fi->config->input_vector->value.vector;
    } else {
        // No input vector specfified, look for the first vector in the parameter list.
        for(int i = 0; i < n_params; ++i) {
            if(fi->params[i].type == VECTOR_TYPE || fi->params[i].type == FUNCTION_TYPE) {
                input = &fi->params[i];
                break;
            }
        }
        switch(input->type) {
            case FUNCTION_TYPE:
                for(int i = 0; i < input->value.function.n_params; ++i) {
                    if(input->value.function.params[i].type != VECTOR_TYPE) continue;
                    input_vec = &input->value.function.params[i].value.vector;
                    input = &input->value.function.params[i];
                    break;
                }
                break;
            case VECTOR_TYPE:
                input_vec = &input->value.vector;
                break;
            default:
                break;
        }
    }

    #ifndef SGX_DEBUG

    output = &fi->params[n_params];
    output_vec = &output->value.vector;

    #else

    sgx_assert(input_vec != NULL, "ERROR, input parameter contains no vector. Undefined behaviour from now on..");
    output = &fi->params[n_params];
    switch(output->type) {
        case VECTOR_TYPE:
            output_vec = &output->value.vector;
            break;
        default:
            sgx_assert(true, "ERROR, return parameter is not of vector type. Undefined behaviour from now...");
    }
    sgx_assert(output_vec != NULL, "ERROR, return parameter is not of vector type. Undefined behaviour from now...");

    #endif

    unsigned int len = *input_vec->len;
    int results[len];
    evaluate_predicate_batch(fi->config->postfix, fi->config->postfix_length, hotcall_config, len, results, 0);
    *(output_vec->len) = copy_filtered_results(output_vec, input_vec, len, results);
}
