#ifndef _H_HOTCALL_DO_WHILE
#define _H_HOTCALL_DO_WHILE

#include "hotcall_utils.h"
#include "hotcall_function.h"

#define ESCAPE(...) __VA_ARGS__

#define _DO_WHILE(SM_CTX, ID, CONFIG, CONDITION_ARGS, BODY_ARGS) \
    struct parameter CAT2(DO_WHILE_CONDITION_ARG_,ID)[] = CONDITION_ARGS; \
    struct parameter CAT2(DO_WHILE_BODY_ARG_,ID)[] = BODY_ARGS; \
    struct do_while_config CAT2(DO_WHILE_CONFIG_,ID) = CONFIG;\
    CAT2(DO_WHILE_CONFIG_,ID).body_n_params = sizeof(CAT2(DO_WHILE_BODY_ARG_,ID)) / sizeof(struct parameter); \
    CAT2(DO_WHILE_CONFIG_,ID).condition_n_params = sizeof(CAT2(DO_WHILE_CONDITION_ARG_,ID)) / sizeof(struct parameter); \
    hotcall_bundle_do_while(SM_CTX, &CAT2(DO_WHILE_CONFIG_,ID), CAT2(DO_WHILE_BODY_ARG_,ID), CAT2(DO_WHILE_CONDITION_ARG_,ID))

#define DO_WHILE(CONFIG, CONDITION_ARGS, BODY_ARGS) \
    _DO_WHILE(_sm_ctx, UNIQUE_ID, CONFIG, ESCAPE(CONDITION_ARGS), ESCAPE(BODY_ARGS))


#define CONDITION_PARAMS(...) { __VA_ARGS__ }

#define FUNCTION_PARAMS(...) { __VA_ARGS__ }

struct do_while_config {
    const uint8_t function_id;
    const char *condition_fmt;
    unsigned int body_n_params;
    unsigned int condition_n_params;

};

struct hotcall_do_while {
    struct parameter *body_params;
    struct parameter *condition_params;
    struct do_while_config *config;
};

#endif