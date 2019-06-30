#include "if.h"
#include "predicate.h"

void
hotcall_handle_if(struct ecall_queue_item *qi, const struct hotcall_config *hotcall_config, struct queue_context *queue_ctx, struct batch_status * batch_status) {
    struct if_config *config = qi->call.tif.config;
    unsigned int loop_stack_pos = queue_ctx->loop_stack_pos, queue_position = *queue_ctx->queue_pos;
    unsigned int offset = loop_stack_pos > 0 ? queue_ctx->loop_stack[loop_stack_pos - 1].offset : 0;
    int res = evaluate_predicate(config->postfix, config->postfix_length, hotcall_config, offset);
    if(res && config->else_branch_len > 0) {
        queue_ctx->else_len[queue_ctx->if_nesting] = config->else_branch_len;

    } else if(!res) {
        if(config->then_branch_len > 0) {
            *queue_ctx->queue_pos += config->then_branch_len;
        }
        if(config->return_if_false) {
            batch_status->exit_batch = true;
        }
    }
    queue_ctx->if_nesting++;
}
