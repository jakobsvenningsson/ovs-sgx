#ifndef _H_FUNCTION_BUNDLER_
#define _H_FUNCTION_BUNDLER_

#include <stdbool.h>
#include <stdint.h>

#define HOTCALL_MAX_ARG 25

enum parameter_type { FUNCTION_TYPE, VARIABLE_TYPE, POINTER_TYPE, POINTER_TYPE_v2, VECTOR_TYPE, STRUCT_TYPE, VECTOR_TYPE_v2 };

#define CONFIG(...) ((struct hotcall_function_config)  { __VA_ARGS__ })

#define VAR(VAL, ...) (struct parameter) { .type = VARIABLE_TYPE, .value = { .variable = { .arg = &(VAL), __VA_ARGS__ }}}
#define VAR_v2(VAL, ...) (struct parameter) { .type = VARIABLE_TYPE, .value = { .variable = { .arg = (VAL), __VA_ARGS__ }}}

#define PTR(VAL, ...) (struct parameter) { .type = POINTER_TYPE,  .value = { .pointer = { .arg = (void *) (VAL), __VA_ARGS__ }}}
#define PTR_v2(VAL, ...) (struct parameter) { .type = POINTER_TYPE_v2,  .value = { .pointer_v2 = { .arg = (VAL), __VA_ARGS__ }}}


#define VECTOR(...) (struct parameter) { .type = VECTOR_TYPE, .value = { .vector = { __VA_ARGS__ }}}
#define VECTOR_v2(VAL,...) (struct parameter) { .type = VECTOR_TYPE_v2, .value = { .vector_v2 = { .arg = (VAL), __VA_ARGS__ }}}

#define FUNC(...) (struct parameter) { .type = FUNCTION_TYPE, .value = { .function = { __VA_ARGS__ }}}
#define STRUCT(VAL, ...) (struct parameter) { .type = STRUCT_TYPE, .value = { .struct_ = { .arg = (VAL), __VA_ARGS__ }}}

#define _HCALL(SM_CTX, ID, CONFIG, ...) \
    struct parameter CAT2(HCALL_ARGS_, ID)[] = { __VA_ARGS__ }; \
    struct hotcall_function_config CAT2(HCALL_CONFIG_, ID) = CONFIG; \
    CAT2(HCALL_CONFIG_, ID).n_params = sizeof(CAT2(HCALL_ARGS_, ID))/sizeof(struct parameter);\
    struct ecall_queue_item CAT2(QUEUE_ITEM_, ID) = { 0 };\
    CAT2(QUEUE_ITEM_, ID).type = QUEUE_ITEM_TYPE_FUNCTION;\
    struct hotcall_function *ID;\
    ID = &(CAT2(QUEUE_ITEM_, ID)).call.fc;\
    ID->config = &CAT2(HCALL_CONFIG_, ID);\
    ID->params = CAT2(HCALL_ARGS_, ID);\
    if(!(SM_CTX)->hcall.batch) { \
        SM_CTX->hcall.ecall = &CAT2(QUEUE_ITEM_, ID);\
        make_hotcall(&(SM_CTX)->hcall);\
    } else if(!(SM_CTX)->hcall.batch->queue) {\
        SM_CTX->hcall.batch->queue = &CAT2(QUEUE_ITEM_, ID);\
        SM_CTX->hcall.batch->top = &CAT2(QUEUE_ITEM_, ID);\
    } else {\
        CAT2(QUEUE_ITEM_, ID).prev = (SM_CTX)->hcall.batch->top;\
        SM_CTX->hcall.batch->top->next = &CAT2(QUEUE_ITEM_, ID);\
        SM_CTX->hcall.batch->top = &CAT2(QUEUE_ITEM_, ID);\
    }


#define HCALL(CONFIG, ...) \
    _HCALL(_sm_ctx, UNIQUE_ID, CONFIG, __VA_ARGS__)


struct variable_parameter {
    void *arg;
    char fmt;
    bool dereference;
    int member_offset;
};

struct function_parameter {
    uint8_t function_id;
    struct parameter *params;
    unsigned int n_params;
};

struct pointer_parameter {
    void *arg;
    char fmt;
    bool dereference;
    unsigned int member_offset;
};

struct parameter;

struct struct_parameter {
    struct parameter *arg;
    unsigned int member_offset;
    unsigned int struct_size;
};

struct pointer_parameter_v2 {
    struct parameter *arg;
    bool dereference;
};

struct vector_parameter {
    void *arg;
    char fmt;
    unsigned int *len;
    bool dereference;
    int member_offset;
    unsigned int *vector_offset;
};

struct vector_parameter_v2 {
    struct parameter *arg;
    unsigned int *len;
    unsigned int offset;
};

union parameter_types {
    struct variable_parameter variable;
    struct function_parameter function;
    struct pointer_parameter pointer;
    struct pointer_parameter_v2 pointer_v2;
    struct vector_parameter vector;
    struct vector_parameter_v2 vector_v2;
    struct struct_parameter struct_;

};

struct parameter {
    enum parameter_type type;
    union parameter_types value;
};


struct hotcall_function_config {
    const uint8_t function_id;
    const bool has_return;
    const bool async;
    unsigned int n_params;
};

struct hotcall_function {
    struct parameter *params;
    struct hotcall_function_config *config;
    //void *return_value;
    void *args[1][HOTCALL_MAX_ARG];
};

#endif
