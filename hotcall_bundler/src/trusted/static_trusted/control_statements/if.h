#ifndef _H_TRUSTED_HOTCALL_IF_
#define _H_TRUSTED_HOTCALL_IF_

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <hotcall_if.h>
#include <hotcall_config.h>
#include <hotcall_for.h>
#include <hotcall-bundler-trusted.h>

void
hotcall_handle_if(struct ecall_queue_item *qi, const struct hotcall_config *hotcall_config, struct queue_context *queue_ctx, struct batch_status * batch_status);

static inline void
hotcall_handle_if_else(struct ecall_queue_item *qi, const struct hotcall_config *hotcall_config, struct queue_context *queue_ctx, struct batch_status * batch_status) {
    qi->next = qi->call.tife.config->if_end;
}

#endif
