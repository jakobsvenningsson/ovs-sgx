#ifndef _H_FUNCTION_
#define _H_FUNCTION_

#define HOTCALL_MAX_ARG 25

#include "hotcall.h"

enum parameter_type { FUNCTION_TYPE_, VARIABLE_TYPE_, POINTER_TYPE_ };

#define _INIT_FUNCTION_ARG(ID, ARG_LIST, FUNCTION, ...)\
    uint8_t F_ ## ID = hotcall_ ## FUNCTION; \
    struct f_argument ARGS_ ## ID[] = { __VA_ARGS__ }; \
    unsigned int N_ARGS_ ## ID = sizeof(ARGS_ ## ID)/sizeof(void *);\
    void *TMP_ ## ID[] = { &F_ ## ID, &N_ARGS_ ## ID, (void *) ARGS_ ## ID};\
    ARG_LIST = TMP_ ## ID

#define INIT_FUNCTION_ARG(ARG_LIST, FUNCTION, ...)\
    _INIT_FUNCTION_ARG(UNIQUE_ID, ARG_LIST, FUNCTION, __VA_ARGS__)


struct variable_parameter {
    void *arg;
    char fmt;
    bool iter;
};

struct function_parameter {
    uint8_t f_id;
    struct parameter *params;
    unsigned int n_params;
};

struct pointer_parameter {
    void *arg;
    bool iter;
};



union parameter_types {
    struct variable_parameter variable;
    struct function_parameter function;
    struct pointer_parameter pointer;

};

struct parameter {
    enum parameter_type type;
    union parameter_types value;
    unsigned int *len;

};

struct hotcall_function_arg_list {
    int n_args;
    void *args[HOTCALL_MAX_ARG];
};

struct hotcall_function_args {
    void *return_value;
    struct hotcall_function_arg_list args;
    bool async;
};
/*
struct function_parameter {
    void *arg;
    char fmt;
    bool iter;
};*/

struct function_parameters_in {
    struct function_parameter *params;
    unsigned int n_params;
    unsigned int *iters;
};

struct hotcall_function_config {
    uint8_t f_id;
    bool has_return;
    bool async;
    unsigned int n_params;
};

struct hotcall_function_ {
    struct parameter *params;
    struct hotcall_function_config *config;
    void *return_value;
};

struct hotcall_function {
    uint8_t id;
    struct hotcall_function_arg_list args;
    void *return_value;
    bool async;
};


#endif
