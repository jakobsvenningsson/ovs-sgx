#ifndef _H_HOTCALL_IF
#define _H_HOTCALL_IF

#include "hotcall_utils.h"
#include "hotcall_function.h"

#define PASTE(X, Y) X ## Y

#define _IF(SM_CTX, ID, CONFIG, ...) \
    struct parameter CAT2(IF_ARG_, ID)[] = { \
        __VA_ARGS__\
    }; \
    struct if_config CAT2(IF_CONFIG_, ID) = CONFIG; \
    struct postfix_item CAT2(POSTFIX_, ID)[strlen(CAT2(IF_CONFIG_, ID).predicate_fmt)];\
    if(!(SM_CTX)->hcall.batch.ignore_hcalls) { \
        hotcall_bundle_if(SM_CTX, &CAT2(IF_CONFIG_, ID), CAT2(IF_ARG_, ID), CAT2(POSTFIX_, ID));\
    }

#define IF(CONFIG, ...) \
    _IF(_sm_ctx, UNIQUE_ID, CONFIG, __VA_ARGS__);

#define THEN
#define ELSE \
    if(!(_sm_ctx)->hcall.batch.ignore_hcalls) { \
        hotcall_bundle_if_else(_sm_ctx);\
    }

#define END \
    if(!(_sm_ctx)->hcall.batch.ignore_hcalls) { \
        hotcall_bundle_if_end(_sm_ctx);\
    }

#define INIT_IF_CONF(FMT, RETURN_IF_FALSE) ((struct if_config) { FMT, RETURN_IF_FALSE })


struct postfix_item {
        char ch;
        struct parameter *elem;
};

struct if_config {
    const char *predicate_fmt;
    const bool return_if_false;
    unsigned int then_branch_len;
    unsigned int else_branch_len;
    struct postfix_item *postfix;
    unsigned int postfix_length;
};

struct hotcall_if {
    struct parameter *params;
    struct if_config *config;
};

#endif
