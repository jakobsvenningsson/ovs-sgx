#ifndef _H_TRUSTED_HOTCALL_ASSERT_
#define _H_TRUSTED_HOTCALL_ASSERT_

#include <hotcall_assert.h>
#include <hotcall-bundler-trusted.h>

static inline void
hotcall_handle_assert(struct ecall_queue_item *qi, const struct hotcall_config *hotcall_config, struct queue_context *queue_ctx, struct batch_status * batch_status) {
    struct hotcall_assert *as = &qi->call.as;
    int res;

    switch(as->param->type) {
        case VARIABLE_TYPE:
            res = *(int *) as->param->value.variable.arg;
            break;
        case FUNCTION_TYPE:
        {
            void *args[1][as->param->value.function.n_params];
            parse_function_arguments(as->param->value.function.params, as->param->value.function.n_params, 0, args);
            execute_function(hotcall_config, as->param->value.function.function_id, 1, as->param->value.function.n_params, args);
            res = *(int *) args[0][as->param->value.function.n_params - 1];
            break;
        }
        default: SWITCH_DEFAULT_REACHED
    }

    if(res != as->config->expected) {
        batch_status->error = as->config->error_code;
        batch_status->exit_batch = true;
    }
}

#endif
