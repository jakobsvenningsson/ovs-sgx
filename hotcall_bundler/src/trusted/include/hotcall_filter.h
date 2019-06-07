#ifndef _H_HOTCALL_FILTER_
#define _H_HOTCALL_FILTER_

#include "hotcall_function.h"
#include "hotcall_predicate.h"

#define _FILTER(SM_CTX, ARGS, UNIQUE_ID) \
    struct filter_args UNIQUE_ID = ARGS; \
    hotcall_bundle_filter(SM_CTX, 0, &UNIQUE_ID)

#define FILTER(SM_CTX, ARGS) \
    _FILTER(SM_CTX, ARGS, UNIQUE_ID)

struct function_filter_out {
    struct function_parameter *params;
    unsigned int *len;
};
struct filter_args {
    struct function_parameters_in *params_in;
    struct function_parameters_in *params_out;
    struct predicate predicate;
};
struct hotcall_filter {
    uint8_t f;
    struct filter_args *args;
};

#endif
