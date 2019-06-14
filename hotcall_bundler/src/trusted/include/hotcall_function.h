#ifndef _H_FUNCTION_
#define _H_FUNCTION_

#define HOTCALL_MAX_ARG 25

#include "hotcall.h"

enum parameter_type { FUNCTION_TYPE, VARIABLE_TYPE, POINTER_TYPE, VECTOR_TYPE };

#define CONFIG(...) ((struct hotcall_functionconfig)  { __VA_ARGS__ })

struct variable_parameter {
    void *arg;
    char fmt;
};

struct vector_parameter {
    void *arg;
    char fmt;
    unsigned int *len;
};

struct function_parameter {
    uint8_t function_id;
    struct parameter *params;
    unsigned int n_params;
};

struct pointer_parameter {
    void **arg;
    char fmt;
};

union parameter_types {
    struct variable_parameter variable;
    struct function_parameter function;
    struct pointer_parameter pointer;
    struct vector_parameter vector;
};

struct parameter {
    enum parameter_type type;
    union parameter_types value;
};

struct hotcall_functionconfig {
    const uint8_t function_id;
    const bool has_return;
    const bool async;
    unsigned int n_params;
};

struct hotcall_function {
    struct parameter *params;
    struct hotcall_functionconfig *config;
    void *return_value;
    void *args[HOTCALL_MAX_ARG];
};

#endif
