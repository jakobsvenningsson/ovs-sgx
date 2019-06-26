#ifndef _H_HOTCALL_WHILE
#define _H_HOTCALL_WHILE

#include "hotcall_utils.h"
#include "hotcall_function.h"

#define END_WHILE() \
    hotcall_bundle_while_end(_sm_ctx)


#define _BEGIN_WHILE(SM_CTX, ID, CONFIG, ...) \
    struct parameter CAT2(FILTER_ARG_,ID[]) = { \
        __VA_ARGS__\
    }; \
    struct while_config CAT2(WHILE_CONFIG_,ID) = CONFIG;\
    CAT2(WHILE_CONFIG_,ID).n_params = sizeof(CAT2(FILTER_ARG_,ID))/sizeof(struct parameter);\
    struct postfix_item CAT2(POSTFIX_, ID)[strlen(CAT2(WHILE_CONFIG_, ID).predicate_fmt)];\
    hotcall_bundle_while_begin(SM_CTX, &CAT2(WHILE_CONFIG_,ID), CAT2(FILTER_ARG_, ID), CAT2(POSTFIX_, ID))

#define BEGIN_WHILE(CONFIG, ...) \
    _BEGIN_WHILE(_sm_ctx, UNIQUE_ID, CONFIG, __VA_ARGS__)

struct while_config {
    const char *predicate_fmt;
    bool iter_vectors;
    bool loop_in_process;
    unsigned int body_len;
    unsigned int n_params;
    struct postfix_item *postfix;
    unsigned int postfix_length;
};

struct hotcall_while_start {
    struct while_config *config;
    struct parameter *params;
};

struct hotcall_while_end {};

#endif
