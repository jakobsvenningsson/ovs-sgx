#ifndef _H_EXAMPLES_H
#define _H_EXAMPLES_H
#include "hotcall.h"

#define N_FOR_ITERS 10

void hotcall_bundle_example_if(struct shared_memory_ctx *sm_ctx);
void hotcall_bundle_example_for(struct shared_memory_ctx *sm_ctx);
void hotcall_bundle_example_while(struct shared_memory_ctx *sm_ctx);
void hotcall_bundle_example_for_each(struct shared_memory_ctx *sm_ctx);
void hotcall_bundle_example_filter(struct shared_memory_ctx *sm_ctx);
void hotcall_bundle_example_map(struct shared_memory_ctx *sm_ctx);
void hotcall_bundle_example_do_while(struct shared_memory_ctx *sm_ctx);

#endif
