#ifndef _H_HOTCALL_UTILS_
#define _H_HOTCALL_UTILS_

#define CAT3(a, b, c) a ## b ## c
#define CAT2(a, b) a ## b
#define CAT(a, b, c) CAT3(a, b, c)
#define UNIQUE_ID CAT(_uid_, __LINE__,  __func__)

#define SWITCH_DEFAULT_REACHED printf("Default reached at %s %d\n", __FILE__, __LINE__);

#define ui8 'x'
#define ui16 'y'
#define ui32 'z'

#endif
