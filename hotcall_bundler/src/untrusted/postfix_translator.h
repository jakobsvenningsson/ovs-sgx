#ifndef _H_HOTCALL_UNTRUSTED_POSTFIX_TRANSLATOR_
#define _H_HOTCALL_UNTRUSTED_POSTFIX_TRANSLATOR_

#include "hotcall_if.h"

unsigned int
to_postfix(const char *condition_fmt, struct parameter *predicate_args, struct postfix_item *output);

#endif
