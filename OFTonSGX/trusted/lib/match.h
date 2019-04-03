/*
 * match.h
 *
 *  Created on: Apr 23, 2018
 *      Author: jorge
 */

#ifndef ENCLAVE_MYENCLAVE_TRUSTED_LIB_MATCH_H_
#define ENCLAVE_MYENCLAVE_TRUSTED_LIB_MATCH_H_

#include "flow.h"

/* A flow classification match.
 *
 * Use one of the match_*() functions to initialize a "struct match".
 *
 * The match_*() functions below maintain the following important invariant.
 * If a bit or a field is wildcarded in 'wc', then the corresponding bit or
 * field in 'flow' is set to all-0-bits.  (The match_zero_wildcarded_fields()
 * function can be used to restore this invariant after adding wildcards.) */
struct match {
    struct flow flow;
    struct flow_wildcards wc;
};


struct minimatch {
    struct miniflow flow;
    struct minimask mask;
};




void minimatch_init(struct minimatch *, const struct match *);
void minimatch_clone(struct minimatch *, const struct minimatch *);
void minimatch_expand(const struct minimatch *, struct match *);
void minimatch_destroy(struct minimatch *);
bool minimatch_equal(const struct minimatch *a, const struct minimatch *b);
uint32_t minimatch_hash(const struct minimatch *, uint32_t basis);





#endif /* ENCLAVE_MYENCLAVE_TRUSTED_LIB_MATCH_H_ */
