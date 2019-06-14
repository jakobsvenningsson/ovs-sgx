#ifndef _HOTCALL_BUNDLER_SAMPLE_H_
#define _HOTCALL_BUNDLER_SAMPLE_H_

#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

#if defined(__cplusplus)
extern "C" {
#endif

void printf(const char *fmt, ...);
void
execute_function(uint8_t function_id, void *args[], void *return_value);

#if defined(__cplusplus)
}
#endif

#endif /* !_HOTCALL_BUNDLER_SAMPLE_H_ */
