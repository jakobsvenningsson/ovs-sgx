#ifndef _H_HOTCALL_FILTER_
#define _H_HOTCALL_FILTER_

#include "hotcall_utils.h"
#include "hotcall_function.h"

#define _FILTER(SM_CTX, ID, CONFIG, ...) \
    struct parameter CAT2(FILTER_ARG_,ID[]) = { \
        __VA_ARGS__\
    }; \
    struct filter_config CAT2(FILTER_CONFIG_,ID) = CONFIG;\
    CAT2(FILTER_CONFIG_,ID).n_params = sizeof(CAT2(FILTER_ARG_,ID))/sizeof(struct parameter);\
    hotcall_bundle_filter(SM_CTX, &CAT2(FILTER_CONFIG_,ID), CAT2(FILTER_ARG_, ID))

#define FILTER(CONFIG, ...) \
    _FILTER(_sm_ctx, UNIQUE_ID, CONFIG, __VA_ARGS__)


struct filter_config {
    const char *condition_fmt;
    uint8_t n_params;
};

struct hotcall_filter {
    struct parameter *params;
    struct filter_config *config;
};

#endif
