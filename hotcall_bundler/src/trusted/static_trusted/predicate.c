#include "predicate.h"
#include <string.h>

int
evaluate_predicate(struct postfix_item *predicate_postfix, unsigned int postfix_length, const struct hotcall_config *hotcall_config, int offset) {
    int res;
    evaluate_predicate_batch(predicate_postfix, postfix_length, hotcall_config, 1, &res, offset);
    return res;
}
