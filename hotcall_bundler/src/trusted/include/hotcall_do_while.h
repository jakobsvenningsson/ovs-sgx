#ifndef _H_HOTCALL_DO_WHILE
#define _H_HOTCALL_DO_WHILE

#include "hotcall_function.h"
#include "hotcall_predicate.h"

#define DO_WHILE(SM_CTX, F, ARGS) \
    hotcall_bundle_do_while((SM_CTX), hotcall_ ## F, ARGS)

struct do_while_args {
    struct function_parameters_in *params;
    struct predicate predicate;
};

struct hotcall_do_while {
    uint8_t f;
    struct do_while_args *args;
};

#endif
