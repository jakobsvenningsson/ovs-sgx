#ifndef _H_HOTCALL_ERROR
#define _H_HOTCALL_ERROR

struct hotcall_error {
    int error_code;
};

#define _ERROR(SM_CTX, ERROR_CODE) \
    hotcall_bundle_error(SM_CTX, ERROR_CODE)
#define ERROR(ERROR_CODE) \
    _ERROR(_sm_ctx, ERROR_CODE)
#endif
