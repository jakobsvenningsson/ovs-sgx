#include "boolean_expression_translator.h"
#include <string.h>

static inline int
evaluate_function(struct function_parameter *param, char ch, struct hotcall_config *hotcall_config, int offset) {
    unsigned int n_params = param->n_params;
    void *args[n_params];
    parse_function_arguments(param->params, n_params, offset, args);
    int operand = 0;
    hotcall_config->execute_function(param->function_id, args, &operand);
    switch(ch) {
        case 'p': return operand;
        case 'b': return ((bool) operand);
        case 'd': return operand;
        case 'u': case ui8: case ui16: case ui32:
            return ((unsigned int) operand);
        default:
            SWITCH_DEFAULT_REACHED
    }
}

static inline int
evaluate_pointer(struct pointer_parameter *param) {
    if(param->arg == NULL) return 0;
    else return 1;
}

static inline int
evaluate_variable(struct variable_parameter *param, char ch) {
    switch(ch) {
        case 'p': return (void *) param->arg == NULL;
        case 'b': return *(bool *) param->arg;
        case 'd': return *(int *) param->arg;
        case 'u': case ui8: case ui16: case ui32: return *(unsigned int *) param->arg;
        default:
            SWITCH_DEFAULT_REACHED
    }
}

#define OFFSET(ARG, TYPE, OFFSET) ((TYPE) ARG + OFFSET)
#define OFFSET_DEREF(ARG, TYPE, OFFSET) *((TYPE) ARG + OFFSET)

static inline int
evaluate_vector(struct vector_parameter *param, char ch, int offset) {
    void *arg = param->arg;
    if(param->dereference) {
        arg = ((char *) OFFSET_DEREF(param->arg, void **, offset)) + param->member_offset;
        offset = 0;
    }
    switch(ch) {
        case 'p': return *((void **) arg + offset) == NULL;
        case 'b': return *((bool *) arg+ offset);
        case 'd': return *((int *) arg + offset);
        case 'u': case ui8: case ui16: case ui32: return *((unsigned int *) arg+ offset);
        default:
            SWITCH_DEFAULT_REACHED
    }
}

int
evaluate_parameter(struct postfix_item *operand_item, struct hotcall_config *hotcall_config, int offset) {
    switch(operand_item->elem->type) {
        case FUNCTION_TYPE: return evaluate_function(&operand_item->elem->value.function, operand_item->ch, hotcall_config, offset);
        case POINTER_TYPE: return evaluate_pointer(&operand_item->elem->value.pointer);
        case VECTOR_TYPE: return evaluate_vector(&operand_item->elem->value.vector, operand_item->ch, offset);
        case VARIABLE_TYPE: return evaluate_variable(&operand_item->elem->value.variable, operand_item->ch);
        default:
            SWITCH_DEFAULT_REACHED
    }
}

static int inline
_evaluate_postfix(struct postfix_item *postfix, unsigned int output_length, struct hotcall_config *hotcall_config, int offset) {

    struct postfix_item *output[output_length], *it;
    unsigned int stack_pos = 0;

    int operand1, operand2, res;

    char stack[output_length];
    int input[output_length];

    enum input_type { OPERATOR, OPERAND };
    enum input_type type[output_length];

    for(int i = 0; i < output_length; ++i) {
        if(!(postfix[i].ch >= 'a' && postfix[i].ch <= 'z')) {
            input[i] = postfix[i].ch;
            type[i] = OPERATOR;
        }
        else {
            input[i] = evaluate_parameter(&postfix[i], hotcall_config, offset);
            type[i] = OPERAND;
        }
    }

    for(int i = 0; i < output_length; ++i) {
        if(type[i] == OPERAND || input[i] == '!') {
            stack[stack_pos++] = input[i];
            continue;
        }

        operand2 = stack[--stack_pos];
        if(operand2 == '!') {
            operand2 = !stack[--stack_pos];
        }


        operand1 = stack[--stack_pos];
        if(operand1 == '!') {
            operand1 = !stack[--stack_pos];
        }


        switch(input[i]) {
            case '&':
                stack[stack_pos++] = operand1 && operand2;
                break;
            case '|':
                stack[stack_pos++] = operand1 || operand2;
                break;
            case '>':
                stack[stack_pos++] = operand1 > operand2;
                break;
            case '<':
                stack[stack_pos++] = operand1 < operand2;
                break;
            case '%':
                stack[stack_pos++] = operand1 % operand2;
                break;
            default:
                SWITCH_DEFAULT_REACHED
        }
    }

    operand1 = stack[--stack_pos];
    if(operand1 == '!') operand1 = !stack[--stack_pos];
    return operand1;
}


int
evaluate_postfix(struct postfix_item *postfix, unsigned int output_length, struct hotcall_config *hotcall_config, int offset) {
    return _evaluate_postfix(postfix, output_length, hotcall_config, offset);
}


int
evaluate_postfix_batch(struct postfix_item *postfix, unsigned int output_length, struct hotcall_config *hotcall_config, int n, int result[n]) {
    for(int i = 0; i < n; ++i) {
        result[i] = _evaluate_postfix(postfix, output_length, hotcall_config, i);
    }
}

void
copy_filtered_results(struct vector_parameter *output_vec, struct parameter *input, int *results, unsigned int len) {
    int n_include = 0;
    for(int n = 0; n < len; ++n) {
        if(results[n]) {
            switch(output_vec->fmt) {
                case 'u':
                    ((unsigned int *) output_vec->arg)[n_include] = *(unsigned int *) parse_argument_v2(input, n);
                    break;
                case 'b':
                    ((bool *) output_vec->arg)[n_include] = *(bool *) parse_argument_v2(input, n);
                    break;
                case 'd':
                    ((int *) output_vec->arg)[n_include] = *(int *) parse_argument_v2(input, n);
                    break;
                case 'p':
                    ((void **) output_vec->arg)[n_include] = *(void **) parse_argument_v2(input, n);
                    break;
                default:
                    SWITCH_DEFAULT_REACHED
                    break;
                }
            n_include++;
        }
    }
    *(output_vec->len) =  n_include;
}
