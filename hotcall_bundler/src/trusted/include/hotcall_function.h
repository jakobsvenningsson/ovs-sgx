#ifndef _H_FUNCTION_
#define _H_FUNCTION_

#define HOTCALL_MAX_ARG 25

struct function_parameter {
    void *arg;
    char fmt;
    bool iter;
};

struct function_parameters_in {
    struct function_parameter *params;
    unsigned int n_params;
    unsigned int iters;
};

typedef struct {
    int n_args;
    void *args[HOTCALL_MAX_ARG];
} argument_list;

struct hotcall_function {
    uint8_t id;
    argument_list args;
    void *return_value;
    bool async;
};


#endif
