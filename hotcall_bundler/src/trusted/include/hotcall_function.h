#ifndef _H_FUNCTION_
#define _H_FUNCTION_

#define HOTCALL_MAX_ARG 25

#define CAT2(a, b, c) a ## b ## c
#define CAT(a, b, c) CAT2(a, b, c)
#define UNIQUE_ID CAT(_uid_, __LINE__,  __func__)

struct hotcall_function_arg_list {
    int n_args;
    void *args[HOTCALL_MAX_ARG];
};

struct hotcall_function_args {
    void *return_value;
    struct hotcall_function_arg_list args;
    bool async;
};

struct function_parameter {
    void *arg;
    char fmt;
    bool iter;
};

struct function_parameters_in {
    struct function_parameter *params;
    unsigned int n_params;
    unsigned int *iters;
};



struct hotcall_function {
    uint8_t id;
    struct hotcall_function_arg_list args;
    void *return_value;
    bool async;
};


#endif
