#ifndef _H_TRUSTED_HOTCALL_FOR_
#define _H_TRUSTED_HOTCALL_FOR_

#include <math.h>
#include <stdint.h>
#include <string.h>
#include <hotcall_for.h>
#include <hotcall-bundler-trusted.h>


static inline unsigned int
update_loop_body_vector_variables(struct loop_stack_item *loop_stack, unsigned int loop_stack_pos) {
    unsigned int offset = 0, power = 0;
    for(int k = loop_stack_pos; k >= 0; --k) {
        offset += loop_stack[k].index * pow(loop_stack[k].n_iters, power++);
    }
    return offset;
}


static inline void
hotcall_handle_for_begin(struct ecall_queue_item *qi, const struct hotcall_config *hotcall_config, struct queue_context *queue_ctx, struct batch_status * batch_status) {
    struct hotcall_for_start *for_s = &qi->call.for_s;
    unsigned int loop_stack_pos = queue_ctx->loop_stack_pos;
    struct loop_stack_item *loop_stack = queue_ctx->loop_stack;

    unsigned int n_iters = *for_s->config->n_iters;
    if(n_iters == 0) {
        qi->next = for_s->config->loop_end;
    } else {
        loop_stack[loop_stack_pos] = (struct loop_stack_item) {
            .loop_start = qi->next,
            .index = 0,
            .n_iters = n_iters,
        };
        loop_stack[loop_stack_pos].offset = update_loop_body_vector_variables(loop_stack, loop_stack_pos);
        queue_ctx->loop_stack_pos++;
    }
}

static inline void
hotcall_handle_for_end(struct ecall_queue_item *qi, const struct hotcall_config *hotcall_config, struct queue_context *queue_ctx, struct batch_status * batch_status) {
    struct loop_stack_item *loop_stack = queue_ctx->loop_stack;
    unsigned int loop_stack_pos = queue_ctx->loop_stack_pos;

    if(!loop_stack[loop_stack_pos - 1].index) {
        loop_stack[loop_stack_pos - 1].loop_end = qi->next;
    }

    if(++loop_stack[loop_stack_pos - 1].index < loop_stack[loop_stack_pos - 1].n_iters) {
        qi->next = loop_stack[loop_stack_pos - 1].loop_start;
        loop_stack[loop_stack_pos - 1].offset++;
    } else {
        qi->next = loop_stack[loop_stack_pos - 1].loop_end;
        queue_ctx->loop_stack_pos--;
    }
}

#endif
