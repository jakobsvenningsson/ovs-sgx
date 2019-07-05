#include "postfix_translator.h"
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
            output[output_index++] = (struct postfix_item) { input[i], &predicate_args[variable_index++] };
            continue;
        }

        switch(input[i]) {
            case '(': chs[pos++] = input[i]; break;
            case ')':
                while(pos > 0 && (tmp = chs[pos - 1]) != '(') {
                    output[output_index++] = (struct postfix_item) { tmp };
                    pos--;
                }
                if(pos > 0 && tmp == '(') {
                    pos--;
                }
                break;
            default:
                while(pos > 0 && chs[pos - 1] != '(' && presedence(input[i]) <= presedence(chs[pos - 1])) {
                    tmp = chs[--pos];
                    output[output_index++] = (struct postfix_item) { tmp };
                }
                chs[pos++] = input[i];
        }
    }

    while(pos > 0) {
        tmp = chs[--pos];
        output[output_index++].ch = tmp;
    }
    return output_index;
}*/
