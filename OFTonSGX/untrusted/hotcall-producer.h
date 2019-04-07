#ifndef _H_HOTCALL_PRODUCER_H
#define _H_HOTCALL_PRODUCER_H

#include "common.h"
#include "stdbool.h"

static async_ecall ctx;

void compile_arg_list(void **return_val, argument_list *arg_list, bool has_return, int n_args, ...);
void make_hotcall(async_ecall * ctx, int function, argument_list * args, void * ret);
//void cleanup_hotcall(char *fmt, argument_list * arg_list);

#endif
