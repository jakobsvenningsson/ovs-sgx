#ifndef _H_HOTCALL_PRODUCER_H
#define _H_HOTCALL_PRODUCER_H

#include "common.h"

static async_ecall ctx;

argument_list * compile_arg_list(char *fmt, void **ret, int n_args, ...);
void make_hotcall(async_ecall * ctx, int function, argument_list * args, void * ret);

#endif
