#include "do_while.h"
#include "predicate.h"
#include "hotcall_function.h"
#include "parameter.h"


void
hotcall_handle_do_while(struct hotcall_do_while *dw, struct hotcall_config *hotcall_config) {
    struct hotcall_function fc;
    struct hotcall_function_config config = {
        .function_id = dw->config->function_id,
        .n_params = dw->config->body_n_params
    };
    fc.config = &config;
    parse_function_arguments(dw->body_params, config.n_params, 0, fc.args);
    while(true) {
        if(!evaluate_predicate(dw->config->postfix, dw->config->postfix_length, hotcall_config, 0)) {
            return;
        }
        hotcall_config->execute_function(dw->config->function_id, (void **) fc.args, NULL);
    }
}
