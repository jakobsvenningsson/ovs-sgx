#ifndef _H_HOTCALL_MAP
#define _H_HOTCALL_MAP

#include "hotcall_function.h"

#define _MAP(SM_CTX, ID, CONFIG, ...) \
    struct parameter CAT2(MAP_ARG_,ID)[] = { \
        __VA_ARGS__\
    }; \
    struct map_config CAT2(MAP_CONFIG_,ID) = CONFIG;\
    CAT2(MAP_CONFIG_,ID).n_params = sizeof(CAT2(MAP_ARG_,ID))/sizeof(struct parameter);\
    hotcall_bundle_map(SM_CTX, &CAT2(MAP_CONFIG_,ID), CAT2(MAP_ARG_, ID))

#define MAP(SM_CTX, CONFIG, ...) \
    _MAP(SM_CTX, UNIQUE_ID, CONFIG, __VA_ARGS__)


struct map_config {
    uint8_t function_id;
    unsigned int n_params;
};
struct hotcall_map {
    struct parameter *params;
    struct map_config *config;
};

#endif
