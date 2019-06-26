#include "boolean_expression_translator.h"
#include <string.h>


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

#define OFFSET(ARG, TYPE, OFFSET) ((TYPE) ARG + OFFSET)
#define OFFSET_DEREF(ARG, TYPE, OFFSET) *((TYPE) ARG + OFFSET)

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
evaluate_function_v3(struct function_parameter *fun_param, struct hotcall_config *hotcall_config, int n_iters, int inputs[n_iters], int offset) {
    unsigned int n_params = fun_param->n_params;
    void *args[n_iters][n_params];
    parse_arguments(fun_param->params, n_params, n_iters, args, offset);
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
_evaluate_postfix(unsigned int output_length, int n_iters, int inputs[output_length][n_iters], enum input_type input_types[output_length], int result[n_iters]) {
    unsigned int stack_pos = 0;
    int *operand1, *operand2;
    enum input_type op1_type, op2_type;

    int *stack[output_length];
    enum input_type type_stack[output_length];

    int input;
    int idx1, idx2;

    for(int i = 0; i < output_length; ++i) {
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
evaluate_postfix_batch(struct postfix_item *postfix, unsigned int output_length, struct hotcall_config *hotcall_config, int n, int result[n], int offset) {

    int inputs[output_length][n];
    enum input_type input_types[output_length];

    for(int i = 0; i < output_length; ++i) {
        if(!isalpha(postfix[i].ch)) {
            inputs[i][0] = postfix[i].ch;
            input_types[i] = postfix[i].ch == '!' ? NEGATION : OPERATOR;
            continue;
        }
        switch(postfix[i].elem->type) {
            case VARIABLE_TYPE:
                inputs[i][0] = evaluate_variable(&postfix[i].elem->value.variable, postfix[i].ch);
                input_types[i] = VARIABLE;
                break;
            case POINTER_TYPE:
                inputs[i][0] = evaluate_pointer(&postfix[i].elem->value.pointer);
                input_types[i] = POINTER;
                break;
            case VECTOR_TYPE:
                for(int j = 0; j < n; ++j) {
                    inputs[i][j] = evaluate_vector(&postfix[i].elem->value.vector, postfix[i].ch, j + offset);
                }
                input_types[i] = VECTOR;
                break;
            case FUNCTION_TYPE:
                evaluate_function_v3(&postfix[i].elem->value.function, hotcall_config, n, inputs[i], offset);
                input_types[i] = FUNCTION;
                break;
        }
    }
    if(output_length == 1) {
        memcpy(result, inputs[0], sizeof(int) * n);
    } else {
        _evaluate_postfix(output_length, n, inputs, input_types, result);
    }
}

int
evaluate_postfix(struct postfix_item *postfix, unsigned int output_length, struct hotcall_config *hotcall_config, int offset) {
    int res[1];
    evaluate_postfix_batch(postfix, output_length, hotcall_config, 1, res, offset);
    return res[0];
}

void
copy_filtered_results(struct vector_parameter *output_vec, struct vector_parameter *input_vec, unsigned int len, int results[len]) {
    int n_include = 0;
    for(int n = 0; n < len; ++n) {
        if(results[n]) {
            switch(output_vec->fmt) {
                case 'u':
                    ((unsigned int *) output_vec->arg)[n_include] = ((unsigned int *) input_vec->arg)[n];
                    break;
                case 'b':
                    ((bool *) output_vec->arg)[n_include] = ((bool *) input_vec->arg)[n];
                    break;
                case 'd':
                    ((int *) output_vec->arg)[n_include] = ((int *) input_vec->arg)[n];
                    break;
                case 'p':
                    ((void **) output_vec->arg)[n_include] = ((void **) input_vec->arg)[n];
                    break;
                default: SWITCH_DEFAULT_REACHED
            }
            n_include++;
        }
    }
    *(output_vec->len) =  n_include;
}
