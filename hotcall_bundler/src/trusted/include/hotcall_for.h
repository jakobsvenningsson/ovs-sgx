#ifndef _H_HOTCALL_FOR
#define _H_HOTCALL_FOR

#include "hotcall_function.h"

#define BEGIN_FOR(SM_CTX, ARGS) \
    hotcall_bundle_for_begin(SM_CTX, ARGS)
#define END_FOR(SM_CTX, ARGS) \
    hotcall_bundle_for_end(SM_CTX, ARGS)

struct for_args {
    unsigned int n_iters;
    unsigned int n_rows;
};
struct hotcall_for_start {
    struct for_args *args;
};
struct hotcall_for_end {
    struct for_args *args;
};

#endif
