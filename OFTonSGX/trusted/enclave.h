#ifndef _MYENCLAVE_H_
#define _MYENCLAVE_H_

#include <stdlib.h>
#include <assert.h>
#include "ofproto-provider.h"
#include "enclave-batch-allocator.h"




#define OFPROTO_FOR_EACH_TABLE(TABLE, SGX_TABLES, SGX_N_TABLES)              \
    for ((TABLE) = SGX_TABLES;                       \
         (TABLE) < &SGX_TABLES[SGX_TABLES, SGX_N_TABLES]; \
         (TABLE)++)


 #define FOR_EACH_MATCHING_TABLE(BRIDGE_ID, TABLE, TABLE_ID, OFPROTO, E_CTX)         \
     for ((TABLE) = first_matching_table(E_CTX, BRIDGE_ID, OFPROTO, TABLE_ID);       \
          (TABLE) != NULL;                                         \
          (TABLE) = next_matching_table(E_CTX,BRIDGE_ID, OFPROTO, TABLE, TABLE_ID))


#if defined(__cplusplus)
extern "C" {
#endif

struct ovs_enclave_ctx {
    struct oftable * SGX_oftables[5];
    struct SGX_table_dpif * SGX_table_dpif[5];
    int SGX_n_tables[5];
    struct sgx_cls_table * SGX_hmap_table[5];
    struct batch_allocator cr_ba;
    struct batch_allocator evg_ba;
};
//extern struct ovs_enclave_ctx e_ctx;


void printf(const char *fmt, ...);

#if defined(__cplusplus)
}
#endif

#endif /* !_MYENCLAVE_H_ */
