#ifndef _MYENCLAVE_H_
#define _MYENCLAVE_H_

#include <stdlib.h>
#include <assert.h>
#include "ofproto-provider.h"

/*
struct oftable *SGX_oftables[100];
struct SGX_table_dpif * SGX_table_dpif[100];
int SGX_n_tables[100];
struct sgx_cls_table * SGX_hmap_table[100];*/


#define OFPROTO_FOR_EACH_TABLE(TABLE, SGX_TABLES)              \
    for ((TABLE) = SGX_TABLES;                       \
         (TABLE) < &SGX_TABLES[SGX_n_tables[bridge_id]]; \
         (TABLE)++)


 #define FOR_EACH_MATCHING_TABLE(BRIDGE_ID, TABLE, TABLE_ID, OFPROTO)         \
     for ((TABLE) = first_matching_table(BRIDGE_ID, OFPROTO, TABLE_ID);       \
          (TABLE) != NULL;                                         \
          (TABLE) = next_matching_table(BRIDGE_ID, OFPROTO, TABLE, TABLE_ID))


#if defined(__cplusplus)
extern "C" {
#endif

void printf(const char *fmt, ...);

#if defined(__cplusplus)
}
#endif

#endif /* !_MYENCLAVE_H_ */
