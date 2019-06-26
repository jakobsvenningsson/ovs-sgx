#ifndef _H_LIB_HOTCALL_TRUSTED_
#define _H_LIB_HOTCALL_TRUSTED_

#ifdef __cplusplus
extern "C" {
#endif

#include "hotcall_config.h"

#define SWITCH_DEFAULT_REACHED printf("Default reached at %s %d\n", __FILE__, __LINE__);
#define MAX_LOOP_RECURSION 3

static struct hotcall_config *hotcall_config;

void
hotcall_register_config(struct hotcall_config *config);

#ifdef __cplusplus
}
#endif

#endif
