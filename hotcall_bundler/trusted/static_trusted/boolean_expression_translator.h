#ifndef _H_BOOLEAN_EXPRESSION_H
#define _H_BOOLEAN_EXPRESSION_H


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
to_postfix(struct transaction_if *tif, struct postfix_item *output, unsigned int *output_length) {
    char chs[128];
    unsigned int pos = 0, output_index = 0, variable_index = 0;
    char tmp;
    char *input = tif->args->fmt;
    for(int i = 0; i < strlen(input); ++i) {
        if(input[i] >= 'a' && input[i] <= 'z') {
            output[output_index].ch = input[i];
            output[output_index].var = &tif->args->variables[variable_index++];
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
evaluate_variable(struct postfix_item *operand_item, struct hotcall_config *hotcall_config) {
    switch(operand_item->var->type) {
        case FUNCTION_TYPE:
        {
            struct function_call *fc;
            fc = (struct function_call *) operand_item->var->val;
            int operand;
            fc->return_value = &operand;
            hotcall_config->execute_function(fc);
            return operand;
        }
        case POINTER_TYPE:
            return operand_item->var->val == NULL;
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
evaluate_postfix(struct transaction_if *tif, struct postfix_item *postfix, unsigned int output_length, struct hotcall_config *hotcall_config) {

    struct postfix_item output[128];
    unsigned int stack_pos = 0;

    struct postfix_item it;

    int operand1, operand2, res;
    struct postfix_item operand1_item, operand2_item;

    for(int i = 0; i < output_length; ++i) {
        it = postfix[i];

        if(it.ch >= 'a' && it.ch <= 'z') {
            output[stack_pos++] = it;
            continue;
        }

        if(it.ch == '!') {
            struct postfix_item *pi;
            pi = &output[stack_pos - 1];
            switch(pi->ch) {
                case 'b':
                    *(bool *) pi->var->val = *(bool *) pi->var->val == 0 ? 1 : 0;
                    break;
                case 'u':
                    *(unsigned int *) pi->var->val = *(unsigned int *) pi->var->val == 0 ? 1 : 0;
                    break;
                case 'd':
                    *(int *) pi->var->val = *(int *) pi->var->val == 0 ? 1 : 0;
                    break;
                default:
                    printf("Default reached at %s %d %c\n", __FILE__, __LINE__, pi->ch);
            }
            continue;
        }

        operand2_item = output[--stack_pos];
        operand2 = evaluate_variable(&operand2_item, hotcall_config);
        operand1_item = output[--stack_pos];
        operand1 = evaluate_variable(&operand1_item, hotcall_config);

        switch(it.ch) {
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
        output[stack_pos].ch = 'b';
        output[stack_pos].var->type = VARIABLE_TYPE;
        output[stack_pos].var->val = &res;
        stack_pos++;
    }

    return *(int *) output[--stack_pos].var->val;
}

#endif
