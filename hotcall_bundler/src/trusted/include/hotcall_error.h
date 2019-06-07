#ifndef _H_HOTCALL_ERROR
#define _H_HOTCALL_ERROR

struct hotcall_error {
    int error_code;
};

#define ERROR(SM_CTX, ERROR_CODE) \
    hotcall_bundle_error(SM_CTX, ERROR_CODE)

#endif
