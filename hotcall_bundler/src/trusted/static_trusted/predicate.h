#ifndef _H_BOOLEAN_EXPRESSION_H
#define _H_BOOLEAN_EXPRESSION_H

#include "hotcall_config.h"
#include "hotcall_function.h"
#include "hotcall_utils.h"
#include <stddef.h>
#include "parameter.h"
#include "execute_function.h"
#include "hotcall_if.h"



#define OFFSET(ARG, TYPE, OFFSET) ((TYPE) ARG + OFFSET)
#define OFFSET_DEREF(ARG, TYPE, OFFSET) *((TYPE) ARG + OFFSET)
/*
int
evaluate_predicate(struct postfix_item *predicate_postfix, unsigned int postfiX_length, const struct hotcall_config *hotcall_config, int offset);*/

enum input_type { OPERATOR, VARIABLE, VECTOR, POINTER, OPERAND, FUNCTION, NEGATION };
struct input_item {
    int ch;
    enum input_type type;
};


static inline bool
op_greater_than(unsigned int n_iters, int x[n_iters], int y[n_iters], int result[n_iters], bool iter_x, bool iter_y) {
    if(iter_x && iter_y) for(int i = 0; i < n_iters; ++i) result[i] = x[i] > y[i];
    else if(iter_x && !iter_y) for(int i = 0; i < n_iters; ++i) result[i] = x[i] > y[0];
    else if(!iter_x && iter_y) for(int i = 0; i < n_iters; ++i) result[i] = x[0] > y[i];
    else for(int i = 0; i < n_iters; ++i) result[i] = x[0] > y[0];}

static inline bool
op_smaller_than(unsigned int n_iters, int x[n_iters], int y[n_iters], int result[n_iters], bool iter_x, bool iter_y) {
    if(iter_x && iter_y) for(int i = 0; i < n_iters; ++i) result[i] = x[i] < y[i];
    else if(iter_x && !iter_y) for(int i = 0; i < n_iters; ++i) result[i] = x[i] < y[0];
    else if(!iter_x && iter_y) for(int i = 0; i < n_iters; ++i) result[i] = x[0] < y[i];
    else for(int i = 0; i < n_iters; ++i) result[i] = x[0] < y[0];
}

static inline bool
op_modulus(unsigned int n_iters, int x[n_iters], int y[n_iters], int result[n_iters], bool iter_x, bool iter_y) {
    if(iter_x && iter_y) for(int i = 0; i < n_iters; ++i) result[i] = x[i] % y[i];
    else if(iter_x && !iter_y) for(int i = 0; i < n_iters; ++i) result[i] = x[i] % y[0];
    else if(!iter_x && iter_y) for(int i = 0; i < n_iters; ++i) result[i] = x[0] % y[i];
    else for(int i = 0; i < n_iters; ++i) result[i] = x[0] % y[0];
}

static inline bool
op_logical_or(unsigned int n_iters, int x[n_iters], int y[n_iters], int result[n_iters], bool iter_x, bool iter_y) {
    if(iter_x && iter_y) for(int i = 0; i < n_iters; ++i) result[i] = x[i] || y[i];
    else if(iter_x && !iter_y) for(int i = 0; i < n_iters; ++i) result[i] = x[i] || y[0];
    else if(!iter_x && iter_y) for(int i = 0; i < n_iters; ++i) result[i] = x[0] || y[i];
    else for(int i = 0; i < n_iters; ++i) result[i] = x[0] || y[0];
}

static inline bool
op_logical_and(unsigned int n_iters, int x[n_iters], int y[n_iters], int result[n_iters], bool iter_x, bool iter_y) {
    if(iter_x && iter_y) for(int i = 0; i < n_iters; ++i) result[i] = x[i] && y[i];
    else if(iter_x && !iter_y) for(int i = 0; i < n_iters; ++i) result[i] = x[i] && y[0];
    else if(!iter_x && iter_y) for(int i = 0; i < n_iters; ++i) result[i] = x[0] && y[i];
    else for(int i = 0; i < n_iters; ++i) result[i] = x[0] && y[0];
}

static inline void
_evaluate_predicate(unsigned int postfix_length, int n_iters, int inputs[postfix_length][n_iters], enum input_type input_types[postfix_length], int result[n_iters]) {

    unsigned int stack_pos = 0;
    int *operand1, *operand2;
    enum input_type op1_type, op2_type;

    int *stack[postfix_length];
    enum input_type type_stack[postfix_length];

    int input;
    int idx1, idx2;

    for(int i = 0; i < postfix_length; ++i) {
        switch(input_types[i]) {
            case FUNCTION: case VECTOR: case VARIABLE: case POINTER: case NEGATION:
                type_stack[stack_pos] = input_types[i];
                stack[stack_pos++] = inputs[i];
                continue;
            case OPERATOR:
                input = inputs[i][0];
                break;
            default: SWITCH_DEFAULT_REACHED
        }
        operand2 = stack[--stack_pos];
        op2_type = type_stack[stack_pos];
        if(*operand2 == '!' && type_stack[stack_pos] == NEGATION) {
            operand2 = stack[--stack_pos];
            op2_type = type_stack[stack_pos];
            for(int j = 0; j < n_iters; ++j) operand2[j] = !operand2[j];
        }
        operand1 = stack[--stack_pos];
        op1_type = type_stack[stack_pos];
        if(*operand1 == '!' && type_stack[stack_pos] == NEGATION) {
            operand1 = stack[--stack_pos];
            op1_type = type_stack[stack_pos];
            for(int j = 0; j < n_iters; ++j) operand1[j] = !operand1[j];
        }

        bool (*op_func)(unsigned int, int[], int[], int[], bool, bool);
        switch(input) {
            case '>': op_func = op_greater_than; break;
            case '<': op_func = op_smaller_than; break;
            case '&': op_func = op_logical_and; break;
            case '|': op_func = op_logical_or; break;
            case '%': op_func = op_modulus; break;
            default: SWITCH_DEFAULT_REACHED
        }
        op_func(n_iters, operand1, operand2, result, op1_type == VECTOR || op1_type == FUNCTION, op2_type == VECTOR || op2_type == FUNCTION);

        type_stack[stack_pos] = VECTOR;
        stack[stack_pos++] = result;
    }

    operand1 = stack[--stack_pos];
    if(*operand1 == '!') {
        operand1 = stack[--stack_pos];
        for(int i = 0; i < n_iters; ++i) result[i] = !operand1[i];
    }
}


static inline int
evaluate_pointer(const struct pointer_parameter *param) {
    void *arg = (param->dereference)
        ? ((char *) *((void **) param->arg)) + param->member_offset
        : ((char *) param->arg) + param->member_offset;
    return arg != NULL;
}

static inline int
evaluate_variable(const struct variable_parameter *param, char ch) {
    void *arg = (param->dereference)
        ? ((char *) *((void **) param->arg)) + param->member_offset
        : ((char *) param->arg) + param->member_offset;

    switch(ch) {
        case 'p': return *(void **) arg != NULL;
        case 'b': return *(bool *) arg;
        case 'd': return *(int *) arg;
        case 'u': return *(unsigned int *) arg;
        case ui8: return *(uint8_t *) arg;
        case ui16: return *(uint16_t *) arg;
        case ui32: return *(uint32_t *) arg;
        case ui64: return *(uint64_t *) arg;
        default:
            SWITCH_DEFAULT_REACHED
    }
}

static inline void
evaluate_int_vector(int *int_vec, unsigned int n_iters, int inputs[n_iters], int offset) {
    for(int i = 0; i < n_iters; ++i) {
        inputs[i] = int_vec[offset + i];
    }
}

static inline void
evaluate_bool_vector(bool *bool_vec, unsigned int n_iters, int inputs[n_iters], int offset) {
    for(int i = 0; i < n_iters; ++i) {
        inputs[i] = bool_vec[offset + i];
    }
}

static inline void
evaluate_ptr_vector(void **ptr_vec, unsigned int n_iters, int inputs[n_iters], int offset) {
    for(int i = 0; i < n_iters; ++i) {
        inputs[i] = ptr_vec[offset + i] != NULL;
    }
}

static inline void
dereference_ptr2int_vector(void **vec, unsigned int n_iters, int inputs[n_iters], unsigned int offset, int member_offset) {
    for(int i = 0; i < n_iters; ++i) {
        inputs[i] = *(int *) (((char *) OFFSET_DEREF(vec, void **, offset + i)) + member_offset);
    }
}

static inline void
dereference_ptr2bool_vector(void **vec, unsigned int n_iters, int inputs[n_iters], unsigned int offset, int member_offset) {
    for(int i = 0; i < n_iters; ++i) {
        inputs[i] = *(bool *) (((char *) OFFSET_DEREF(vec, void **, offset + i)) + member_offset);
    }
}

static inline void
dereference_ptr2ptr_vector(void **vec, unsigned int n_iters, int inputs[n_iters], unsigned int offset, int member_offset) {
    for(int i = 0; i < n_iters; ++i) {
        inputs[i] = *(void **) (((char *) OFFSET_DEREF(vec, void **, offset + i)) + member_offset) != NULL;
    }
}


static inline void
evaluate_vector(const struct vector_parameter *vec_param, unsigned int offset, unsigned int n_iters, int inputs[n_iters]) {
    unsigned int member_offset = vec_param->member_offset;
    bool dereference = vec_param->dereference;
    void *arg = vec_param->arg;
    switch(vec_param->fmt) {
        case 'd': case 'u':
            dereference
                ? dereference_ptr2int_vector(arg, n_iters, inputs, offset, member_offset)
                : evaluate_int_vector(arg, n_iters, inputs, offset);
            break;
        case 'b':
            dereference
                ? dereference_ptr2bool_vector(vec_param->arg, n_iters, inputs, offset, member_offset)
                : evaluate_bool_vector(arg, n_iters, inputs, offset);
            break;
        case 'p':
            dereference
                ? dereference_ptr2ptr_vector(vec_param->arg, n_iters, inputs, offset, member_offset)
                : evaluate_ptr_vector(arg, n_iters, inputs, offset);
            break;
        default:
            SWITCH_DEFAULT_REACHED
    }
}

static inline void
evaluate_function(struct function_parameter *fun_param, const struct hotcall_config *hotcall_config, int n_iters, int inputs[n_iters], int offset) {
    unsigned int n_params = fun_param->n_params;
    void *args[n_params][n_iters];
    parse_arguments(fun_param->params, n_iters, n_params, args, offset);
    execute_function(hotcall_config, fun_param->function_id, n_iters, n_params, args);
    for(int k = 0; k < n_iters; ++k) {
        inputs[k] = *(bool *) args[n_params - 1][k];
    }
}

static inline void
evaluate_predicate_batch(struct postfix_item *predicate_postfix, unsigned int postfix_length, const struct hotcall_config *hotcall_config, int n, int result[n], int offset) {
    int inputs[postfix_length][n];
    enum input_type input_types[postfix_length];

    for(int i = 0; i < postfix_length; ++i) {
        if(!(predicate_postfix[i].ch >= 'a' && predicate_postfix[i].ch <= 'z')) {
            inputs[i][0] = predicate_postfix[i].ch;
            input_types[i] = predicate_postfix[i].ch == '!' ? NEGATION : OPERATOR;
            continue;
        }
        switch(predicate_postfix[i].elem->type) {
            case VARIABLE_TYPE:
                if(postfix_length == 1) {
                    result[i] = evaluate_variable(&predicate_postfix[i].elem->value.variable, predicate_postfix[i].ch);
                } else {
                    inputs[i][0] = evaluate_variable(&predicate_postfix[i].elem->value.variable, predicate_postfix[i].ch);
                }
                input_types[i] = VARIABLE;
                break;
            case POINTER_TYPE:
                if(postfix_length == 1) {
                    result[i] = evaluate_pointer(&predicate_postfix[i].elem->value.pointer);
                } else {
                    inputs[i][0] = evaluate_pointer(&predicate_postfix[i].elem->value.pointer);
                }
                input_types[i] = POINTER;
                break;
            case VECTOR_TYPE:
                if(postfix_length == 1) {
                    evaluate_vector(&predicate_postfix[i].elem->value.vector, offset, n, result);
                } else {
                    evaluate_vector(&predicate_postfix[i].elem->value.vector, offset, n, inputs[i]);
                }
                input_types[i] = VECTOR;
                break;
            case FUNCTION_TYPE:
                if(postfix_length == 1) {
                    evaluate_function(&predicate_postfix[i].elem->value.function, hotcall_config, n, result, offset);
                } else {
                    evaluate_function(&predicate_postfix[i].elem->value.function, hotcall_config, n, inputs[i], offset);
                }
                input_types[i] = FUNCTION;
                break;
        }
    }

    /* If condition has length 1, then the value of the single parameter is the final result
        of the condition hence we can simply copy the input values to our results list. */
    if(postfix_length != 1) {
        _evaluate_predicate(postfix_length, n, inputs, input_types, result);
    }
}

#endif
