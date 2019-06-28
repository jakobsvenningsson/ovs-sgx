#ifndef _H_TRUSTED_HOTCALL_DO_WHILE_
#define _H_TRUSTED_HOTCALL_DO_WHILE_

#include "hotcall_do_while.h"
#include "hotcall_config.h"
#include <hotcall-bundler-trusted.h>

void
hotcall_handle_do_while(struct ecall_queue_item *qi, const struct hotcall_config *hotcall_config, struct queue_context *queue_ctx, struct batch_status * batch_status);

#endif
