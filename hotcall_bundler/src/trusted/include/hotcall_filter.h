#ifndef _H_HOTCALL_FILTER_
#define _H_HOTCALL_FILTER_

#include "hotcall_function.h"
#include "hotcall_predicate.h"
#include "hotcall.h"

/*
#define _FILTER(SM_CTX, ARGS, UNIQUE_ID) \
    struct filter_args UNIQUE_ID = ARGS; \
    hotcall_bundle_filter(SM_CTX, 0, &UNIQUE_ID)

#define FILTER(SM_CTX, ARGS) \
    _FILTER(SM_CTX, ARGS, UNIQUE_ID)*/

#define _FILTER(SM_CTX, ID, CONFIG, ...) \
    struct parameter CAT2(FILTER_ARG_,ID[]) = { \
        __VA_ARGS__\
    }; \
    struct filter_config CAT2(FILTER_CONFIG_,ID) = CONFIG;\
    CAT2(FILTER_CONFIG_,ID).n_params = sizeof(CAT2(FILTER_ARG_,ID))/sizeof(struct parameter);\
    hotcall_bundle_filter(SM_CTX, &CAT2(FILTER_CONFIG_,ID), CAT2(FILTER_ARG_, ID))

#define FILTER(SM_CTX, CONFIG, ...) \
    _FILTER(SM_CTX, UNIQUE_ID, CONFIG, __VA_ARGS__)


struct filter_config {
    const char *condition_fmt;
    uint8_t n_params;
};

/*
struct filter_args {
    struct function_parameters_in *params_in;
    struct function_parameters_in *params_out;
    struct predicate predicate;
};*/

struct hotcall_filter {
    struct parameter *params;
    struct filter_config *config;
};

#endif
