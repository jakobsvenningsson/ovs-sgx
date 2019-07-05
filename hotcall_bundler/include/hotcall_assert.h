#ifndef _H_HOTCALL_ASSERT
#define _H_HOTCALL_ASSERT

#include "hotcall_utils.h"
#include "hotcall_function.h"

#define _ASSERT(SM_CTX, ID, EXPECTED, ERROR_CODE, ...) \
    struct parameter CAT2(ASSERT_PARAM_, ID) = __VA_ARGS__; \
    struct assert_config CAT2(ASSERT_CONFIG_, ID) = { ERROR_CODE, EXPECTED }; \
    struct ecall_queue_item CAT2(QUEUE_ITEM_, ID) = { 0 }; \
    hotcall_enqueue_item(SM_CTX, QUEUE_ITEM_TYPE_ASSERT, &CAT2(ASSERT_CONFIG_,ID), &CAT2(ASSERT_PARAM_, ID), &CAT2(QUEUE_ITEM_, ID))

#define ASSERT(EXPECTED, ERROR_CODE, ...) _ASSERT(_sm_ctx, UNIQUE_ID, EXPECTED, ERROR_CODE, __VA_ARGS__)

struct assert_config {
    int error_code;
    bool expected;
};

struct hotcall_assert {
    struct assert_config *config;
    struct parameter *param;

};

#endif
