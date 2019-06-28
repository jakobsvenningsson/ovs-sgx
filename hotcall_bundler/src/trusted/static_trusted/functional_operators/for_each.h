#ifndef _H_TRUSTED_HOTCALL_FOR_EACH_
#define _H_TRUSTED_HOTCALL_FOR_EACH_

#include "hotcall_for_each.h"
#include "hotcall_config.h"
#include <hotcall-bundler-trusted.h>

void
hotcall_handle_for_each(struct ecall_queue_item *qi, const struct hotcall_config *hotcall_config, struct queue_context *queue_ctx, struct batch_status * batch_status);

#endif
