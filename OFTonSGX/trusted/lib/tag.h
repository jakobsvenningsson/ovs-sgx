/*
 * tag.h
 *
 *  Created on: Apr 23, 2018
 *      Author: jorge
 */

#ifndef ENCLAVE_MYENCLAVE_TRUSTED_LIB_TAG_H_
#define ENCLAVE_MYENCLAVE_TRUSTED_LIB_TAG_H_


#include <stdbool.h>
#include <stdint.h>
#include <limits.h>


/* Represents a tag, or the combination of 0 or more tags. */
typedef uint32_t tag_type;

#define N_TAG_BITS (CHAR_BIT * sizeof(tag_type))


/* Adding tags is easy, but subtracting is hard because you can't tell whether
 * a bit was set only by the tag you're removing or by multiple tags.  The
 * tag_tracker data structure counts the number of tags that set each bit,
 * which allows for efficient subtraction. */
struct tag_tracker {
    unsigned int counts[N_TAG_BITS];
};





#endif /* ENCLAVE_MYENCLAVE_TRUSTED_LIB_TAG_H_ */
