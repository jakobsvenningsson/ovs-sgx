#ifndef _H_HOTCALL_WHILE
#define _H_HOTCALL_WHILE

#include "hotcall_function.h"
#include "hotcall_predicate.h"

#define BEGIN_WHILE(SM_CTX, ARGS) \
    hotcall_bundle_while_begin(SM_CTX, ARGS)
#define END_WHILE(SM_CTX, ARGS) \
    hotcall_bundle_while_end(SM_CTX, ARGS)

struct while_args {
    struct predicate predicate;
    unsigned int n_rows;
};
struct hotcall_while_start {
    struct while_args *args;
};
struct hotcall_while_end {
    struct while_args *args;
};

#endif
