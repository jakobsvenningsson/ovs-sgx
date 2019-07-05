#ifndef _H_HOTCALL_ERROR
#define _H_HOTCALL_ERROR

struct error_config {
    int error_code;
};

struct hotcall_error {
    struct error_config *config;
};

#define _ERROR(SM_CTX, ID, ERROR) \
    struct error_config CAT2(ERROR_CONFIG_,ID) = { ERROR };\
    struct ecall_queue_item CAT2(QUEUE_ITEM_, ID) = { 0 }; \
    hotcall_enqueue_item(SM_CTX, QUEUE_ITEM_TYPE_ERROR, &CAT2(ERROR_CONFIG_,ID), NULL, &CAT2(QUEUE_ITEM_, ID))

#define ERROR(ERROR) \
    _ERROR(_sm_ctx, UNIQUE_ID, ERROR)

#define RETURN \
    _ERROR(_sm_ctx, UNIQUE_ID, 0);


//if(!(_sm_ctx)->hcall.batch->ignore_hcalls) {


//}



#endif
