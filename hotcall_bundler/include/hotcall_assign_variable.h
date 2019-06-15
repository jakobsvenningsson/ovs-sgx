#ifndef _H_HOTCALL_ASSIGN_VAR_
#define _H_HOTCALL_ASSIGN_VAR_

#include "hotcall_utils.h"
#include "hotcall_function.h"

#define _ASSIGN_VAR(ID, DST, SRC) \
    struct parameter CAT2(DST_VAR_, ID) = DST;\
    struct parameter CAT2(SRC_VAR_, ID) = SRC;\
    hotcall_bundle_assign_var(_sm_ctx, &CAT2(DST_VAR_, ID), &CAT2(SRC_VAR_, ID))

#define _ASSIGN_PTR(ID, DST, SRC) \
    struct parameter CAT2(DST_PTR_, ID) = DST;\
    struct parameter CAT2(SRC_PTR_, ID) = SRC;\
    hotcall_bundle_assign_ptr(_sm_ctx, &CAT2(DST_PTR_, ID), &CAT2(SRC_PTR_, ID))

#define ASSIGN_VAR(DST, SRC) _ASSIGN_VAR(UNIQUE_ID, (DST), (SRC))
#define ASSIGN_PTR(DST, SRC) _ASSIGN_PTR(UNIQUE_ID, (DST), (SRC))

struct hotcall_assign_variable {
    struct parameter *src;
    struct parameter *dst;
    unsigned int offset;
};

struct hotcall_assign_pointer {
    struct parameter *src;
    struct parameter *dst;
    unsigned int offset;
};

#endif
