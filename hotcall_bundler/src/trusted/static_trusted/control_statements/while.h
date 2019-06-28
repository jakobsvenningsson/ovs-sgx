#ifndef _H_TRUSTED_HOTCALL_WHILE_
#define _H_TRUSTED_HOTCALL_WHILE_

#include "for.h"
#include <string.h>
#include "hotcall_while.h"
#include "hotcall_config.h"
#include <hotcall-bundler-trusted.h>

static inline void
hotcall_handle_while_begin(struct ecall_queue_item *qi, const struct hotcall_config *hotcall_config, struct queue_context *queue_ctx, struct batch_status * batch_status) {
    struct hotcall_while_start *while_s = &qi->call.while_s;
    struct loop_stack_item *loop_stack = queue_ctx->loop_stack;
    unsigned int loop_stack_pos = queue_ctx->loop_stack_pos;

    if(!evaluate_predicate(while_s->config->postfix, while_s->config->postfix_length, hotcall_config, (loop_stack_pos > 0 ? loop_stack[loop_stack_pos - 1].offset : 0 ))) {
        *queue_ctx->queue_pos += while_s->config->body_len + 1;
        queue_ctx->loop_stack_pos--;
        while_s->config->loop_in_process = false;
        return;
    }

    if(!while_s->config->loop_in_process) {
        while_s->config->loop_in_process = true;
        loop_stack[loop_stack_pos].body_len =  while_s->config->body_len;
        loop_stack[loop_stack_pos].offset = (loop_stack_pos > 0) ? loop_stack[loop_stack_pos - 1].offset : 0;
        queue_ctx->loop_stack_pos++;
    } else {
        if(while_s->config->iter_vectors) {
            loop_stack[loop_stack_pos - 1].offset++;
        }
    }
}

static inline void
hotcall_handle_while_end(struct ecall_queue_item *qi, const struct hotcall_config *hotcall_config, struct queue_context *queue_ctx, struct batch_status * batch_status) {
    *queue_ctx->queue_pos -= (queue_ctx->loop_stack[queue_ctx->loop_stack_pos - 1].body_len + 2);
}

#endif
