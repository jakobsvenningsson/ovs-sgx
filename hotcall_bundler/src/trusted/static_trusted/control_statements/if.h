#ifndef _H_TRUSTED_HOTCALL_IF_
#define _H_TRUSTED_HOTCALL_IF_

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <hotcall_if.h>
#include <hotcall_config.h>
#include <hotcall_for.h>
#include <hotcall-bundler-trusted.h>


static inline void
hotcall_handle_if_else(struct ecall_queue_item *qi, const struct hotcall_config *hotcall_config, struct queue_context *queue_ctx, struct batch_status * batch_status) {
    unsigned int else_len = queue_ctx->else_len[queue_ctx->if_nesting - 1];
    if(else_len) *queue_ctx->queue_pos += else_len;
}

static inline void
hotcall_handle_if_end(struct ecall_queue_item *qi, const struct hotcall_config *hotcall_config, struct queue_context *queue_ctx, struct batch_status * batch_status) {
    queue_ctx->else_len[--queue_ctx->if_nesting] = 0;
}


void
hotcall_handle_if(struct ecall_queue_item *qi, const struct hotcall_config *hotcall_config, struct queue_context *queue_ctx, struct batch_status * batch_status);

#endif
