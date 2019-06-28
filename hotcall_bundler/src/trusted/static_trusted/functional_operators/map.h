#ifndef _H_TRUSTED_HOTCALL_MAP_
#define _H_TRUSTED_HOTCALL_MAP_

#include "hotcall_map.h"
#include "hotcall_config.h"
#include <hotcall-bundler-trusted.h>

void
hotcall_handle_map(struct ecall_queue_item *qi, const struct hotcall_config *hotcall_config, struct queue_context *queue_ctx, struct batch_status * batch_status);

#endif
