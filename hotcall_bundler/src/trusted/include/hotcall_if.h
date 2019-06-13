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
#define IF(SM_CTX, CONFIG, ...) \
    _IF(SM_CTX, UNIQUE_ID, CONFIG, __VA_ARGS__);

#define THEN
#define ELSE
#define END

#define INIT_IF_CONF(THEN_LEN, ELSE_LEN, FMT, RETURN_IF_FALSE) ((struct if_config) { THEN_LEN, ELSE_LEN, FMT, RETURN_IF_FALSE })

#define VARIABLE_PARAM(VAL, FMT) { .type = VARIABLE_TYPE_, .value = { .variable = { .arg = VAL, .fmt = FMT, .iter = false }}}
#define POINTER_PARAM(VAL, FMT) { .type = POINTER_TYPE_, .value = { .variable = { .arg = VAL, .iter = false }}}
#define CONFIG(...) ((struct hotcall_function_config)  __VA_ARGS__ )

struct if_config {
    unsigned int then_branch_len;
    unsigned int else_branch_len;
    //struct predicate predicate;
    const char *predicate_fmt;
    bool return_if_false;
    //struct parameter* predicate_args;
};

struct hotcall_if {
    struct parameter *args;
    struct if_config *config;
};

#endif
