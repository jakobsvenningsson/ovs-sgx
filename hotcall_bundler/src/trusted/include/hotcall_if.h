#ifndef _H_HOTCALL_IF
#define _H_HOTCALL_IF

#include "hotcall_function.h"
#include "hotcall_predicate.h"

#define IF(SM_CTX, ARGS) \
    hotcall_bundle_if_(SM_CTX, ARGS);
#define THEN(...) __VA_ARGS__
#define ELSE(...) __VA_ARGS__

struct if_args {
    unsigned int then_branch_len;
    unsigned int else_branch_len;
    struct predicate predicate;
    bool return_if_false;
};

struct hotcall_if {
    struct if_args *args;
};

#endif
