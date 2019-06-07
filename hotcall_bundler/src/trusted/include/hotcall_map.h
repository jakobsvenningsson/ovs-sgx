#ifndef _H_HOTCALL_MAP
#define _H_HOTCALL_MAP

#include "hotcall_function.h"

#define _MAP(SM_CTX, FUNCTION, ARGS, UNIQUE_ID) \
    struct map_args UNIQUE_ID = ARGS; \
    hotcall_bundle_map(SM_CTX, hotcall_ ## FUNCTION,  &UNIQUE_ID)

#define MAP(SM_CTX, FUNCTION, ARGS) \
    _MAP(SM_CTX, FUNCTION, ARGS, UNIQUE_ID)

/*
struct function_map_out {
    void *params;
    char fmt;
};*/
struct map_args {
    struct function_parameters_in *params_in;
    struct function_parameters_in *params_out;
};
struct hotcall_map {
    uint8_t f;
    struct map_args *args;
};

#endif
