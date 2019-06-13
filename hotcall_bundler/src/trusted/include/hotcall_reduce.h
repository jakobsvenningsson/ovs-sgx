#ifndef _H_HOTCALL_REDUCE
#define _H_HOTCALL_REDUCE

#include "hotcall_function.h"

#define _REDUCE(SM_CTX, ID, CONFIG, ...) \
    struct parameter CAT2(REDUCE_ARG_,ID)[] = { \
        __VA_ARGS__\
    }; \
    struct reduce_config CAT2(REDUCE_CONFIG_,ID) = CONFIG;\
    CAT2(REDUCE_CONFIG_,ID).n_params = sizeof(CAT2(REDUCE_ARG_,ID))/sizeof(struct parameter);\
    hotcall_bundle_reduce(SM_CTX, &CAT2(REDUCE_CONFIG_,ID), CAT2(REDUCE_ARG_, ID))

#define REDUCE(CONFIG, ...) \
    _REDUCE(_sm_ctx, UNIQUE_ID, CONFIG, __VA_ARGS__)

struct reduce_config {
    const uint8_t function_id;
    const char op;
    unsigned int n_params;
};

struct hotcall_reduce {
    struct parameter *params;
    struct reduce_config *config;
};

#endif
