#ifndef _H_TEST_
#define _H_TEST_

#include "hotcall.h"

int
hotcall_run_tests(struct shared_memory_ctx *sm_ctx);
void
hotcall_test_setup();
void
hotcall_test_teardown();
struct shared_memory_ctx *
hotcall_test_get_context();

#endif
