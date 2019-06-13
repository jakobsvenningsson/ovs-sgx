#ifndef _H_BOOLEAN_EXPRESSION_H
#define _H_BOOLEAN_EXPRESSION_H

#include "hotcall-trusted.h"

enum postfix_item_type { OPERAND, OPERATOR };
struct postfix_item {
        char ch;
        enum postfix_item_type type;
        struct parameter *elem;
};

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

extern inline void
to_postfix(const char *condition_fmt, struct parameter *predicate_args, struct postfix_item *output, unsigned int *output_length) {
    char chs[128];
    unsigned int pos = 0, output_index = 0, variable_index = 0;
    char tmp;
    const char *input = condition_fmt;
    for(int i = 0; i < strlen(input); ++i) {
        if(input[i] >= 'a' && input[i] <= 'z') {
            output[output_index].ch = input[i];
            output[output_index].elem = &predicate_args[variable_index++];
            output[output_index].type = OPERAND;
            output_index++;
            continue;
        }
        switch(input[i]) {
            case '(':
                chs[pos++] = input[i];
                break;
            case ')':
                while(pos > 0 && (tmp = chs[pos - 1]) != '(') {
                    output[output_index].type = OPERATOR;
                    output[output_index++].ch = tmp;
                    pos--;
                }
                if(pos > 0 && tmp == '(') {
                    pos--;
                }
                break;
            default:
                while(pos > 0 && chs[pos - 1] != '(' && presedence(input[i]) <= presedence(chs[pos - 1])) {
                    tmp = chs[--pos];
                    output[output_index].type = OPERATOR;
                    output[output_index++].ch = tmp;
                }
                chs[pos++] = input[i];
        }
    }

    while(pos > 0) {
        tmp = chs[--pos];
        output[output_index].type = OPERATOR;
        output[output_index++].ch = tmp;
    }
    *output_length = output_index;
}

extern inline int
evaluate_variable(struct postfix_item *operand_item, struct hotcall_config *hotcall_config, int offset) {
    switch(operand_item->elem->type) {
        case FUNCTION_TYPE:
        {
            struct parameter *param = operand_item->elem;
            unsigned int n_params = param->value.function.n_params;
            void *args[n_params];
            for(int j = 0; j < n_params; ++j) args[j] = parse_argument(&param->value.function.params[j], offset);
            int operand = 0;
            hotcall_config->execute_function(operand_item->elem->value.function.function_id, args, &operand);
            switch(operand_item->ch) {
                case 'b':
                    return ((bool) operand);
                case 'd':
                    return operand;
                case 'u':
                    return ((unsigned int) operand);
                default:
                    SWITCH_DEFAULT_REACHED
            }
        }
        case POINTER_TYPE: return operand_item->elem->value.pointer.arg != NULL;
        case VECTOR_TYPE:
            switch(operand_item->ch) {
                case 'b':
                    return *((bool *) operand_item->elem->value.vector.arg + offset);
                case 'u':
                    return *((unsigned int *) operand_item->elem->value.vector.arg + offset);
                case 'd':
                    return *((int *) operand_item->elem->value.vector.arg + offset);
                default:
                    SWITCH_DEFAULT_REACHED
            }
        case VARIABLE_TYPE:
            switch(operand_item->ch) {
                case 'b':
                    return *(bool *) operand_item->elem->value.variable.arg;
                case 'u':
                    return *(unsigned int *) operand_item->elem->value.variable.arg;
                case 'd':
                    return *(int *) operand_item->elem->value.variable.arg;
                default:
                    SWITCH_DEFAULT_REACHED
            }
        default:
            SWITCH_DEFAULT_REACHED
    }
}

extern inline int
evaluate_postfix(struct postfix_item *postfix, unsigned int output_length, struct hotcall_config *hotcall_config, int offset) {

    struct postfix_item *output[output_length];
    unsigned int stack_pos = 0;

    struct postfix_item *it;

    int operand1, operand2, res;

    struct postfix_item *operand1_item, *operand2_item;

    struct parameter p_var = {
        .type = VARIABLE_TYPE,
        .value = { .variable = { .arg = &res }}
    };
    struct postfix_item res_item = {
        .ch = 'b',
        .type = OPERAND,
        .elem = &p_var
    };


    for(int i = 0; i < output_length; ++i) {

        it = &postfix[i];
        if(it->ch >= 'a' && it->ch <= 'z' || it->ch == '!') {
            output[stack_pos++] = it;
            continue;
        }

        operand2_item = output[--stack_pos];
        operand2 = operand2_item->ch == '!'
            ? !evaluate_variable(output[--stack_pos], hotcall_config, offset)
            : evaluate_variable(operand2_item, hotcall_config, offset);

        operand1_item = output[--stack_pos];
        operand1 = operand1_item->ch == '!'
            ? !evaluate_variable(output[--stack_pos], hotcall_config, offset)
            : evaluate_variable(operand1_item, hotcall_config, offset);

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
        ? !evaluate_variable(output[--stack_pos], hotcall_config, offset)
        : evaluate_variable(operand1_item, hotcall_config, offset);
}

#endif
