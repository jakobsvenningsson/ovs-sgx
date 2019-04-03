/*
 * hindex.h
 *
 *  Created on: Apr 23, 2018
 *      Author: jorge
 */

#ifndef ENCLAVE_MYENCLAVE_TRUSTED_LIB_HINDEX_H_
#define ENCLAVE_MYENCLAVE_TRUSTED_LIB_HINDEX_H_

#include <stdbool.h>
#include <stdlib.h>
#include "util.h"



struct hindex_node {
    /* Hash value. */
    size_t hash;

    /* In a head node, the next head node (with a hash different from this
     * node), or NULL if this is the last node in this bucket.
     *
     * In a body node, the previous head or body node (with the same hash as
     * this node).  Never null. */
    struct hindex_node *d;

    /* In a head or a body node, the next body node with the same hash as this
     * node.  NULL if this is the last node with this hash. */
    struct hindex_node *s;
};

/* A hash index. */
struct hindex {
    struct hindex_node **buckets; /* Must point to 'one' iff 'mask' == 0. */
    struct hindex_node *one;
    size_t mask;      /* 0 or more lowest-order bits set, others cleared. */
    size_t n_unique;  /* Number of unique hashes (the number of head nodes). */
};

#endif /* ENCLAVE_MYENCLAVE_TRUSTED_LIB_HINDEX_H_ */
