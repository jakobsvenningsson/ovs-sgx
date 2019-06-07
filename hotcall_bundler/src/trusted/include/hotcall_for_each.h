#ifndef _H_HOTCALL_FOR_EACH
#define _H_HOTCALL_FOR_EACH

#include "hotcall_function.h"

#define FOR_EACH(SM_CTX, FUNCTION, ARGS) hotcall_bundle_for_each(SM_CTX, hotcall_ ## FUNCTION, ARGS)

struct for_each_args {
    struct function_parameters_in *params_in;
};
struct hotcall_for_each {
    uint8_t f;
    struct for_each_args *args;
};

#endif
