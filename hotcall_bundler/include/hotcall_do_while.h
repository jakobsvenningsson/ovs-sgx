#ifndef _H_HOTCALL_DO_WHILE
#define _H_HOTCALL_DO_WHILE

#include "hotcall_utils.h"
#include "hotcall_function.h"

#define ESCAPE(...) __VA_ARGS__

#define _DO_WHILE(SM_CTX, ID, CONFIG, CONDITION_ARGS, BODY_ARGS) \
    struct parameter CAT2(DO_WHILE_CONDITION_ARG_,ID)[] = CONDITION_ARGS; \
    struct parameter CAT2(DO_WHILE_BODY_ARG_,ID)[] = BODY_ARGS; \
    struct do_while_config CAT2(DO_WHILE_CONFIG_,ID) = CONFIG;\
    struct postfix_item CAT2(POSTFIX_, ID)[strlen(CAT2(DO_WHILE_CONFIG_, ID).predicate_fmt)];\
    struct ecall_queue_item CAT2(QUEUE_ITEM_, ID) = { 0 }; \
    CAT2(DO_WHILE_CONFIG_,ID).postfix = CAT2(POSTFIX_, ID);\
    CAT2(DO_WHILE_CONFIG_,ID).body_n_params = sizeof(CAT2(DO_WHILE_BODY_ARG_,ID)) / sizeof(struct parameter); \
    CAT2(DO_WHILE_CONFIG_,ID).condition_n_params = sizeof(CAT2(DO_WHILE_CONDITION_ARG_,ID)) / sizeof(struct parameter); \
    CAT2(DO_WHILE_CONFIG_,ID).condition_params = CAT2(DO_WHILE_CONDITION_ARG_,ID);\
    hotcall_enqueue_item(SM_CTX, QUEUE_ITEM_TYPE_DO_WHILE, &CAT2(DO_WHILE_CONFIG_,ID), CAT2(DO_WHILE_BODY_ARG_,ID), &CAT2(QUEUE_ITEM_, ID));

#define DO_WHILE(CONFIG, CONDITION_ARGS, BODY_ARGS) \
    _DO_WHILE(_sm_ctx, UNIQUE_ID, CONFIG, ESCAPE(CONDITION_ARGS), ESCAPE(BODY_ARGS))


#define CONDITION_PARAMS(...) { __VA_ARGS__ }

#define FUNCTION_PARAMS(...) { __VA_ARGS__ }

struct do_while_config {
    const uint8_t function_id;
    const char *predicate_fmt;
    unsigned int body_n_params;
    unsigned int condition_n_params;
    struct postfix_item *postfix;
    unsigned int postfix_length;
    struct parameter *condition_params;
};

struct hotcall_do_while {
    struct parameter *body_params;
    struct do_while_config *config;
};

#endif
