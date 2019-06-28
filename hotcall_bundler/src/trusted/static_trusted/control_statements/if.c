#include "if.h"
#include "predicate.h"

void
hotcall_handle_if(struct ecall_queue_item *qi, const struct hotcall_config *hotcall_config, struct queue_context *queue_ctx, struct batch_status * batch_status) {
    struct hotcall_if *tif = &qi->call.tif;
    unsigned int loop_stack_pos = queue_ctx->loop_stack_pos, queue_position = *queue_ctx->queue_pos;
    unsigned int offset = loop_stack_pos > 0 ? queue_ctx->loop_stack[loop_stack_pos - 1].offset : 0;
    int res = evaluate_predicate(tif->config->postfix, tif->config->postfix_length, hotcall_config, offset);
    if(res && tif->config->else_branch_len > 0) {
        printf("true\n");
        //exclude_else_branch(queue_ctx->exclude_list, queue_position, tif->config->then_branch_len, tif->config->else_branch_len);
        queue_ctx->else_len[queue_ctx->if_nesting] = tif->config->else_branch_len;

    } else if(!res) {
        if(tif->config->then_branch_len > 0) {
            //exclude_if_branch(queue_ctx->exclude_list, queue_position, tif->config->then_branch_len);
            //memset(exclude_list + pos + 1, 1, if_len);
            *queue_ctx->queue_pos += tif->config->then_branch_len;
        }
        if(tif->config->return_if_false) {
            //exclude_rest(queue_ctx->exclude_list, queue_position, tif->config->then_branch_len ,tif->config->else_branch_len, queue_ctx->len);
            batch_status->exit_batch = true;
        }
    }
    queue_ctx->if_nesting++;
}
