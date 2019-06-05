#ifndef _H_HOTCALL_FILTER_
#define _H_HOTCALL_FILTER_

#include "hotcall_function.h"
#include "hotcall_predicate.h"

#define FILTER(SM_CTX, FUNCTION, ARGS) \
    hotcall_bundle_filter(SM_CTX, hotcall_ ## FUNCTION, ARGS)

struct function_filter_out {
    struct function_parameter *params;
    unsigned int *len;
};
struct filter_args {
    const struct function_parameters_in params_in;
    struct function_filter_out params_out;
    struct predicate predicate;
};
struct hotcall_filter {
    uint8_t f;
    struct filter_args *args;
};

#endif
