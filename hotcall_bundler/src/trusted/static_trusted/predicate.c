#include "predicate.h"
#include <string.h>
#include "parameter.h"

enum input_type { OPERATOR, VARIABLE, VECTOR, POINTER, OPERAND, FUNCTION, NEGATION };
struct input_item {
    int ch;
    enum input_type type;
};

static inline int
evaluate_pointer(struct pointer_parameter *param) {
    void *arg = (param->dereference)
        ? ((char *) *((void **) param->arg)) + param->member_offset
        : ((char *) param->arg) + param->member_offset;
    return arg != NULL;
}

static inline int
evaluate_variable(struct variable_parameter *param, char ch) {
    void *arg = (param->dereference)
        ? ((char *) *((void **) param->arg)) + param->member_offset
        : ((char *) param->arg) + param->member_offset;

    switch(ch) {
        case 'p': return *(void **) arg != NULL;
        case 'b': return *(bool *) arg;
        case 'd': return *(int *) arg;
        case 'u': case ui8: case ui16: case ui32: return *(unsigned int *) arg;
        default:
            SWITCH_DEFAULT_REACHED
    }
}

static inline int
evaluate_vector(struct vector_parameter *param, char ch, int offset) {
    void *arg = param->arg;
    if(param->vector_offset) {
        offset = *param->vector_offset;
    }
    if(param->dereference) {
        arg = ((char *) OFFSET_DEREF(param->arg, void **, offset)) + param->member_offset;
        offset = 0;
    }
    switch(ch) {
        case 'p': return *((void **) arg + offset) != NULL;
        case 'b': return *((bool *) arg+ offset);
        case 'd': return *((int *) arg + offset);
        case 'u': case ui8: case ui16: case ui32: return *((unsigned int *) arg + offset);
        default:
            SWITCH_DEFAULT_REACHED
    }
}

static inline void
evaluate_function(struct function_parameter *fun_param, struct hotcall_config *hotcall_config, int n_iters, int inputs[n_iters], int offset) {
    unsigned int n_params = fun_param->n_params;
    void *args[n_iters][n_params];
    parse_arguments(fun_param->params, n_iters, n_params, args, offset);
    if(hotcall_config->batch_execute_function) {
        hotcall_config->batch_execute_function(fun_param->function_id, n_iters, n_params, args);
    } else {
        for(int j = 0; j < n_iters; ++j) {
               hotcall_config->execute_function(fun_param->function_id, args[j], args[j][n_params - 1]);
        }
    }
    for(int k = 0; k < n_iters; ++k) {
        inputs[k] = *(bool *) args[k][n_params - 1];
    }
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
        if(*operand2 == '!') {
            operand2 = stack[--stack_pos];
            op2_type = type_stack[stack_pos];
            for(int j = 0; j < n_iters; ++j) operand2[j] = !operand2[j];
        }

        operand1 = stack[--stack_pos];
        op1_type = type_stack[stack_pos];
        if(*operand1 == '!') {
            operand1 = stack[--stack_pos];
            op1_type = type_stack[stack_pos];
            for(int j = 0; j < n_iters; ++j) operand1[j] = !operand1[j];
        }
        for(int j = 0; j < n_iters; ++j) {
            idx1 = op1_type == VECTOR || op1_type == FUNCTION ? j : 0;
            idx2 = op2_type == VECTOR || op2_type == FUNCTION ? j : 0;
            switch(input) {
                case '&': result[j] = operand1[idx1] && operand2[idx2]; break;
                case '|': result[j] = operand1[idx1] || operand2[idx2]; break;
                case '>': result[j] = operand1[idx1] > operand2[idx2]; break;
                case '<': result[j] = operand1[idx1] < operand2[idx2]; break;
                case '%': result[j] = operand1[idx1] % operand2[idx2]; break;
                default: SWITCH_DEFAULT_REACHED
            }
        }
        type_stack[stack_pos] = VECTOR;
        stack[stack_pos++] = result;
    }

    operand1 = stack[--stack_pos];
    if(*operand1 == '!') {
        operand1 = stack[--stack_pos];
        for(int i = 0; i < n_iters; ++i) result[i] = !operand1[i];
    }
}

int
evaluate_predicate_batch(struct postfix_item *predicate_postfix, unsigned int postfix_length, struct hotcall_config *hotcall_config, int n, int result[n], int offset) {

    int inputs[postfix_length][n];
    enum input_type input_types[postfix_length];

    for(int i = 0; i < postfix_length; ++i) {
        if(!isalpha(predicate_postfix[i].ch)) {
            inputs[i][0] = predicate_postfix[i].ch;
            input_types[i] = predicate_postfix[i].ch == '!' ? NEGATION : OPERATOR;
            continue;
        }
        switch(predicate_postfix[i].elem->type) {
            case VARIABLE_TYPE:
                inputs[i][0] = evaluate_variable(&predicate_postfix[i].elem->value.variable, predicate_postfix[i].ch);
                input_types[i] = VARIABLE;
                break;
            case POINTER_TYPE:
                inputs[i][0] = evaluate_pointer(&predicate_postfix[i].elem->value.pointer);
                input_types[i] = POINTER;
                break;
            case VECTOR_TYPE:
                for(int j = 0; j < n; ++j) {
                    inputs[i][j] = evaluate_vector(&predicate_postfix[i].elem->value.vector, predicate_postfix[i].ch, j + offset);
                }
                input_types[i] = VECTOR;
                break;
            case FUNCTION_TYPE:
                evaluate_function(&predicate_postfix[i].elem->value.function, hotcall_config, n, inputs[i], offset);
                input_types[i] = FUNCTION;
                break;
        }
    }
    if(postfix_length == 1) {
        memcpy(result, inputs[0], sizeof(int) * n);
    } else {
        _evaluate_predicate(postfix_length, n, inputs, input_types, result);
    }
}

int
evaluate_predicate(struct postfix_item *predicate_postfix, unsigned int postfix_length, struct hotcall_config *hotcall_config, int offset) {
    int res;
    evaluate_predicate_batch(predicate_postfix, postfix_length, hotcall_config, 1, &res, offset);
    return res;
}
