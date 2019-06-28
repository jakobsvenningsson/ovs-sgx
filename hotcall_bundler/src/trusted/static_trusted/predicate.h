#ifndef _H_BOOLEAN_EXPRESSION_H
#define _H_BOOLEAN_EXPRESSION_H

#include "hotcall_config.h"
#include "hotcall_function.h"
#include "hotcall_utils.h"
#include <hotcall_if.h>
#include <stddef.h>

#define OFFSET(ARG, TYPE, OFFSET) ((TYPE) ARG + OFFSET)
#define OFFSET_DEREF(ARG, TYPE, OFFSET) *((TYPE) ARG + OFFSET)

int
evaluate_predicate(struct postfix_item *predicate_postfix, unsigned int postfiX_length, const struct hotcall_config *hotcall_config, int offset);

#endif
