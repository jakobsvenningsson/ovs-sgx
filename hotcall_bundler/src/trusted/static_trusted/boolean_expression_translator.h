#ifndef _H_BOOLEAN_EXPRESSION_H
#define _H_BOOLEAN_EXPRESSION_H

#include "hotcall_config.h"
#include "hotcall_function.h"
#include "hotcall_utils.h"

enum postfix_item_type { OPERAND, OPERATOR };
struct postfix_item {
        char ch;
        enum postfix_item_type type;
        struct parameter *elem;
};

void
to_postfix(const char *condition_fmt, struct parameter *predicate_args, struct postfix_item *output, unsigned int *output_length);

int
evaluate_variable(struct postfix_item *operand_item, struct hotcall_config *hotcall_config, int offset);

int
evaluate_postfix(struct postfix_item *postfix, unsigned int output_length, struct hotcall_config *hotcall_config, int offset);


static inline void *
parse_argument(const struct parameter *param, unsigned int offset) {
    void *arg;
    char fmt;
    unsigned int _offset = 0;
    switch(param->type) {
        case VARIABLE_TYPE: arg = param->value.variable.arg; fmt = param->value.variable.fmt; break;
        case VECTOR_TYPE:
            arg = (void **) param->value.vector.arg; //param->value.vector.dereference ? *(void **) param->value.vector.arg : param->value.vector.arg;
            fmt = param->value.vector.fmt;
            _offset = offset;
            break;
        case POINTER_TYPE:
            arg = (void **) param->value.pointer.arg;// param->value.pointer.dereference ? *(void **) param->value.pointer.arg : param->value.pointer.arg;
            fmt = param->value.pointer.fmt;
            break;
        default: SWITCH_DEFAULT_REACHED
    }
    printf("offset %d %c\n", _offset, fmt);
    switch(fmt) {
        case 'p': return (((void **) arg) + _offset);
        case 'd': return ((int *) arg) + _offset;
        case 'b': return ((bool *) arg) + _offset;
        case 'u': case ui8: case ui16: case ui32:
            return ((unsigned int *) arg) + _offset;
        default: SWITCH_DEFAULT_REACHED
    }
}
/*
static inline void *
parse_argument_1(const struct parameter *param, unsigned int offset, int member_offset) {
    void *arg;
    char fmt;
    unsigned int _offset = 0;
    switch(param->type) {
        case VARIABLE_TYPE: arg = param->value.variable.arg; fmt = param->value.variable.fmt; break;
        case VECTOR_TYPE:
            arg = param->value.vector.dereference ? *((void **) param->value.vector.arg + offset) : param->value.vector.arg;
            fmt = param->value.vector.fmt;
            _offset = offset;
            break;
        case POINTER_TYPE:
            arg = param->value.pointer.dereference ? *(void **) param->value.pointer.arg : param->value.pointer.arg;
            fmt = param->value.pointer.fmt;
            break;
        default: SWITCH_DEFAULT_REACHED
    }
    printf("offset %d %c\n", _offset, fmt);
    switch(fmt) {
        case 'p': return ((char *) arg);
        case 'd': return ((int *) arg) + _offset;
        case 'b': return ((bool *) arg) + _offset;
        case 'u': case ui8: case ui16: case ui32:
            return ((unsigned int *) arg) + _offset;
        default: SWITCH_DEFAULT_REACHED
    }
}*/


static inline void *
load_variable_argument(const struct variable_parameter *var) {
    switch(var->fmt) {
        case 'd': return ((int *) var->arg);
        case 'b': return ((bool *) var->arg);
        case 'u': case ui8: case ui16: case ui32:
            return ((unsigned int *) var->arg);
        default: SWITCH_DEFAULT_REACHED
    }
}


static inline void *
load_vector_argument(const struct vector_parameter *vec, unsigned int offset) {
    switch(vec->fmt) {
        case 'd': return ((int *) vec->arg) + offset;
        case 'b': return ((bool *) vec->arg) + offset;
        case 'u': case ui8: case ui16: case ui32:
            return ((unsigned int *) vec->arg) + offset;
        default: SWITCH_DEFAULT_REACHED
    }
}

#endif
