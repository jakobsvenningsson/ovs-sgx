/*
 * match.c
 *
 *  Created on: Apr 24, 2018
 *      Author: jorge
 */
#include "match.h"
#include <stdlib.h>

//lines 1076
/* Initializes 'dst' as a copy of 'src'.  The caller must eventually free 'dst'
 * with minimatch_destroy(). */
void
minimatch_init(struct minimatch *dst, const struct match *src)
{
    miniflow_init(&dst->flow, &src->flow);
    minimask_init(&dst->mask, &src->wc);
}

/* Initializes 'dst' as a copy of 'src'.  The caller must eventually free 'dst'
 * with minimatch_destroy(). */
void
minimatch_clone(struct minimatch *dst, const struct minimatch *src)
{
    miniflow_clone(&dst->flow, &src->flow);
    minimask_clone(&dst->mask, &src->mask);
}

//lines 1094 -1102
/* Frees any memory owned by 'match'.  Does not free the storage in which
 * 'match' itself resides; the caller is responsible for that. */
void
minimatch_destroy(struct minimatch *match)
{
    miniflow_destroy(&match->flow);
    minimask_destroy(&match->mask);
}

/* Initializes 'dst' as a copy of 'src'. */
void
minimatch_expand(const struct minimatch *src, struct match *dst)
{
    miniflow_expand(&src->flow, &dst->flow);
    minimask_expand(&src->mask, &dst->wc);
}

/* Returns true if 'a' and 'b' match the same packets, false otherwise.  */
bool
minimatch_equal(const struct minimatch *a, const struct minimatch *b)
{
    return (miniflow_equal(&a->flow, &b->flow)
            && minimask_equal(&a->mask, &b->mask));
}

/* Returns a hash value for 'match', given 'basis'. */
uint32_t
minimatch_hash(const struct minimatch *match, uint32_t basis)
{
    return miniflow_hash(&match->flow, minimask_hash(&match->mask, basis));
}


