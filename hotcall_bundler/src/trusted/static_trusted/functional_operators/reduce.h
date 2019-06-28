#ifndef _H_TRUSTED_HOTCALL_REDUCE_
#define _H_TRUSTED_HOTCALL_REDUCE_

#include "hotcall_reduce.h"
#include "hotcall_config.h"
#include <hotcall-bundler-trusted.h>

void
hotcall_handle_reduce(struct ecall_queue_item *qi, const struct hotcall_config *hotcall_config, struct queue_context *queue_ctx, struct batch_status * batch_status);

#endif
