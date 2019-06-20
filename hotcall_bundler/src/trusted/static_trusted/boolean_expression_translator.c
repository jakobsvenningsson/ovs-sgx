#include "boolean_expression_translator.h"
#include <string.h>

/*
static inline
int presedence(char c) {
    switch(c) {
        case '!':
            return 4;
        case '>': case '<': case '%':
            return 3;
        case '&': case '|':
            return 2;
        default:
            return 0;
    }
}

unsigned int
to_postfix(const char *condition_fmt, struct parameter *predicate_args, struct postfix_item *output) {
    char chs[128];
    unsigned int pos = 0, output_index = 0, variable_index = 0;
    char tmp;
    const char *input = condition_fmt;
    for(int i = 0; i < strlen(input); ++i) {

        if(input[i] >= 'a' && input[i] <= 'z') {
            output[output_index++] = (struct postfix_item) { input[i], OPERAND, &predicate_args[variable_index++] };
            continue;
        }

        switch(input[i]) {
            case '(': chs[pos++] = input[i]; break;
            case ')':
                while(pos > 0 && (tmp = chs[pos - 1]) != '(') {
                    output[output_index++] = (struct postfix_item) { tmp, OPERATOR };
                    pos--;
                }
                if(pos > 0 && tmp == '(') {
                    pos--;
                }
                break;
            default:
                while(pos > 0 && chs[pos - 1] != '(' && presedence(input[i]) <= presedence(chs[pos - 1])) {
                    tmp = chs[--pos];
                    output[output_index++] = (struct postfix_item) { tmp, OPERATOR };
                }
                chs[pos++] = input[i];
        }
    }

    while(pos > 0) {
        tmp = chs[--pos];
        output[output_index].type = OPERATOR;
        output[output_index++].ch = tmp;
    }
    return output_index;
}*/


static inline int
evaluate_function(struct function_parameter *param, char ch, struct hotcall_config *hotcall_config, int offset) {
    unsigned int n_params = param->n_params;
    void *args[n_params]; int addr_modifications[10];
    for(int j = 0; j < n_params; ++j) args[j] = parse_argument_1(&param->params[j], addr_modifications, 0, offset);
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

static inline int
evaluate_vector(struct vector_parameter *param, char ch, int offset) {
    switch(ch) {
        case 'p': return *((void **) param->arg + offset) == NULL;
        case 'b': return *((bool *) param->arg + offset);
        case 'd': return *((int *) param->arg + offset);
        case 'u': case ui8: case ui16: case ui32: return *((unsigned int *) param->arg + offset);
        default:
            SWITCH_DEFAULT_REACHED
    }
}

static inline int
evaluate_vector_v2(struct parameter *param, char ch, int offset) {
    int addr_modifications[10];
    void *arg = parse_argument_1(param, addr_modifications, 0, offset);
    switch(ch) {
        case 'p': return (*(void **) arg) == NULL;
        case 'b': return *(bool *)  arg;
        case 'd': return *(int *) arg;
        case 'u': case ui8: case ui16: case ui32: return *(unsigned int *)  arg;
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
        case VECTOR_TYPE_v2: return evaluate_vector_v2(operand_item->elem, operand_item->ch, offset);
        case VARIABLE_TYPE: return evaluate_variable(&operand_item->elem->value.variable, operand_item->ch);
        default:
            SWITCH_DEFAULT_REACHED
    }
}

int
evaluate_postfix(struct postfix_item *postfix, unsigned int output_length, struct hotcall_config *hotcall_config, int offset) {

    struct postfix_item *output[output_length], *it;
    unsigned int stack_pos = 0;

    int operand1, operand2, res;

    /*struct postfix_item res_item = {
        .ch = 'b',
        .elem = &(struct parameter) {
            .type = VARIABLE_TYPE,
            .value = { .variable = { .arg = &res }}
        }
    }, *operand1_item, *operand2_item;*/

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

    /*for(int i = 0; i < output_length; ++i) {

        it = &postfix[i];
        if(it->ch >= 'a' && it->ch <= 'z' || it->ch == '!') {
            output[stack_pos++] = it;
            continue;
        }

        operand2_item = output[--stack_pos];
        operand2 = operand2_item->ch == '!'
            ? !evaluate_parameter(output[--stack_pos], hotcall_config, offset)
            : evaluate_parameter(operand2_item, hotcall_config, offset);

        operand1_item = output[--stack_pos];
        operand1 = operand1_item->ch == '!'
            ? !evaluate_parameter(output[--stack_pos], hotcall_config, offset)
            : evaluate_parameter(operand1_item, hotcall_config, offset);

        switch(it->ch) {
            case '&':
                res = operand1 && operand2;
                break;
            case '|':
                res = operand1 || operand2;
                break;
            case '>':
                res = operand1 > operand2;
                break;
            case '<':
                res = operand1 < operand2;
                break;
            case '%':
                res = operand1 % operand2;
                break;
            default:
                SWITCH_DEFAULT_REACHED
        }
        output[stack_pos++] = &res_item;
    }

    operand1_item = output[--stack_pos];
    return operand1_item->ch == '!'
        ? !evaluate_parameter(output[--stack_pos], hotcall_config, offset)
        : evaluate_parameter(operand1_item, hotcall_config, offset);*/
}
