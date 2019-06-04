#ifndef _H_BOOLEAN_EXPRESSION_H
#define _H_BOOLEAN_EXPRESSION_H

#include "hotcall-trusted.h"

enum postfix_item_type { OPERAND, OPERATOR };
struct postfix_item {
        char ch;
        enum postfix_item_type type;
        struct predicate_variable *var;
};

static inline
int presedence(char c) {
    switch(c) {
        case '!':
            return 4;
        case '>': case '<':
            return 3;
        case '&': case '|':
            return 2;
        default:
            return 0;
    }
}

extern inline void
to_postfix(struct predicate *predicate, struct postfix_item *output, unsigned int *output_length) {
    char chs[128];
    unsigned int pos = 0, output_index = 0, variable_index = 0;
    char tmp;
    char *input = predicate->fmt;
    for(int i = 0; i < strlen(input); ++i) {
        if(input[i] >= 'a' && input[i] <= 'z') {
            output[output_index].ch = input[i];
            output[output_index].var = &predicate->variables[variable_index++];
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



extern unsigned int for_loop_indices[3];
extern unsigned int for_loop_nesting;

extern inline int
evaluate_variable(struct postfix_item *operand_item, struct hotcall_config *hotcall_config) {
    switch(operand_item->var->type) {
        case FUNCTION_TYPE:
        {
            struct function_call *fc;
            fc = (struct function_call *) operand_item->var->val;
            bool operand;
            fc->return_value = &operand;
            //hotcall_config->execute_function(fc);
            hotcall_handle_function(fc);
            return operand;
        }
        case POINTER_TYPE:
            return operand_item->var->val != NULL;
        case VARIABLE_TYPE:
            switch(operand_item->ch) {
                case 'b':
                    return *(bool *) operand_item->var->val;
                case 'u':
                    return *(unsigned int *) operand_item->var->val;
                case 'd':
                    return *(int *) operand_item->var->val;
                default:
                    printf("Default reached at %s %d\n", __FILE__, __LINE__);
            }
        default:
            printf("Default reached at %s %d\n", __FILE__, __LINE__);
    }
}

extern inline int
evaluate_postfix(struct postfix_item *postfix, unsigned int output_length, struct hotcall_config *hotcall_config) {

    struct postfix_item *output[output_length];
    unsigned int stack_pos = 0;

    struct postfix_item *it;

    int operand1, operand2, res;

    struct postfix_item *operand1_item, *operand2_item;

    struct predicate_variable p_var = {
        .type = VARIABLE_TYPE,
        .val = &res
    };
    struct postfix_item res_item = {
        .ch = 'b',
        .type = OPERAND,
        .var = &p_var
    };

    for(int i = 0; i < output_length; ++i) {
        it = &postfix[i];
        if(it->ch >= 'a' && it->ch <= 'z') {
            output[stack_pos++] = it;
            continue;
        }
        if(it->ch == '!') {
            struct postfix_item *operand_item;
            operand_item = output[stack_pos - 1];
            switch(operand_item->var->type) {
                case FUNCTION_TYPE:
                {
                    struct function_call *fc;
                    fc = (struct function_call *) operand_item->var->val;
                    bool operand;
                    fc->return_value = &operand;
                    hotcall_config->execute_function(fc);
                    return !operand;
                }
                case POINTER_TYPE:
                    return operand_item->var->val == NULL;
                case VARIABLE_TYPE:
                    switch(operand_item->ch) {
                        case 'b':
                            return !*(bool *) operand_item->var->val;
                        case 'u':
                            return !*(unsigned int *) operand_item->var->val;
                        case 'd':
                            return !*(int *) operand_item->var->val;
                        default:
                            printf("Default reached at %s %d\n", __FILE__, __LINE__);
                    }
                default:
                    printf("Default reached at %s %d\n", __FILE__, __LINE__);
            }

            continue;
        }


        operand2_item = output[--stack_pos];
        operand2 = evaluate_variable(operand2_item, hotcall_config);

        operand1_item = output[--stack_pos];
        operand1 = evaluate_variable(operand1_item, hotcall_config);


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
            default:
                printf("Default reached at %s %d\n", __FILE__, __LINE__);
        }
        output[stack_pos++] = &res_item;
    }

    operand1_item = output[--stack_pos];
    return evaluate_variable(operand1_item, hotcall_config);
}

#endif
