#ifndef _H_LIB_HOTCALL_TRUSTED_
#define _H_LIB_HOTCALL_TRUSTED_

#include "hotcall.h"

int
ecall_start_poller(struct shared_memory_ctx *sm_ctx);

void
hotcall_register_config(struct hotcall_config *config);

#endif
