#ifndef _H_EXAMPLES_H
#define _H_EXAMPLES_H
#include "hotcall.h"

#define ROUNDS 10000
#define WARM_UP ROUNDS/10
#define ITERATIONS 10
#define N_FOR_ITERS 10

void hotcall_bundle_example_if(struct shared_memory_ctx *sm_ctx);
void hotcall_bundle_example_for(struct shared_memory_ctx *sm_ctx);
void hotcall_bundle_example_while(struct shared_memory_ctx *sm_ctx);

#endif
