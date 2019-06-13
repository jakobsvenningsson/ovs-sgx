#ifndef _H_HOTCALL_IF
#define _H_HOTCALL_IF

#include "hotcall_function.h"
#include "hotcall_predicate.h"

#define PASTE(X, Y) X ## Y

#define _IF(SM_CTX, ID, CONFIG, ...) \
    struct parameter CAT2(IF_ARG_, ID)[] = { \
        __VA_ARGS__\
    }; \
    struct if_config CAT2(IF_CONFIG_, ID) = CONFIG; \
    hotcall_bundle_if_(SM_CTX, &CAT2(IF_CONFIG_, ID), CAT2(IF_ARG_, ID));
#define IF(CONFIG, ...) \
    _IF(_sm_ctx, UNIQUE_ID, CONFIG, __VA_ARGS__);

#define THEN
#define ELSE hotcall_bundle_if_else(_sm_ctx);
#define END hotcall_bundle_if_end(_sm_ctx);

#define INIT_IF_CONF(FMT, RETURN_IF_FALSE) ((struct if_config) { FMT, RETURN_IF_FALSE })




//#define VARIABLE_PARAM(VAL, FMT) { .type = VARIABLE_TYPE_, .value = { .variable = { __VA_ARGS__ }}}
//#define POINTER_PARAM(VAL, FMT) { .type = POINTER_TYPE_, .value = { .variable = { __VA_ARGS__ }}}
#define CONFIG(...) ((struct hotcall_function_config)  { __VA_ARGS__ })

struct if_config {
    //struct predicate predicate;
    const char *predicate_fmt;
    bool return_if_false;
    unsigned int then_branch_len;
    unsigned int else_branch_len;
    //struct parameter* predicate_args;
};

struct hotcall_if {
    struct parameter *args;
    struct if_config *config;
};

#endif
