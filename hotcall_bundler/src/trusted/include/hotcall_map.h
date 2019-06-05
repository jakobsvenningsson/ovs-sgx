#ifndef _H_HOTCALL_MAP
#define _H_HOTCALL_MAP

#include "hotcall_function.h"

#define MAP(SM_CTX, FUNCTION, ARGS) \
    hotcall_bundle_map(SM_CTX, hotcall_ ## FUNCTION, ARGS)


struct function_map_out {
    void *params;
    char fmt;
};
struct map_args {
    const struct function_parameters_in params_in;
    struct function_map_out params_out;
};
struct hotcall_map {
    uint8_t f;
    struct map_args *args;
};

#endif
