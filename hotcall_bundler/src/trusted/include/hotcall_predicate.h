#ifndef _H_HOTCALL_PREDICATE_
#define _H_HOTCALL_PREDICATE_

enum variable_type { FUNCTION_TYPE, VARIABLE_TYPE, POINTER_TYPE };

struct predicate_variable {
    const void *val;
    enum variable_type type;
    char fmt;
};

struct predicate {
    bool expected;
    char *fmt;
    uint8_t n_variables;
    struct predicate_variable *variables;
};

#endif
