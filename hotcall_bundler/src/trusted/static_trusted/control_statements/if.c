#include "if.h"
#include "predicate.h"

void
hotcall_handle_if(struct ecall_queue_item *qi, const struct hotcall_config *hotcall_config, struct queue_context *queue_ctx, struct batch_status * batch_status) {
    struct if_config *config = qi->call.tif.config;
    unsigned int loop_stack_pos = queue_ctx->loop_stack_pos, queue_position = *queue_ctx->queue_pos;
    unsigned int offset = loop_stack_pos > 0 ? queue_ctx->loop_stack[loop_stack_pos - 1].offset : 0;
    int res;
    evaluate_predicate_batch(config->postfix, config->postfix_length, hotcall_config, 1, &res, offset);
    if(res) {
        qi->next = config->then_branch;
    } else if(config->else_branch){
        qi->next = config->else_branch;
        if(config->return_if_false) {
            batch_status->exit_batch = true;
        }
    } else {
        qi->next = config->if_end;
    }
}
