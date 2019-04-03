/*
 * types.h
 *
 *  Created on: Apr 23, 2018
 *      Author: jorge
 */

#ifndef ENCLAVE_MYENCLAVE_TRUSTED_INCLUDE_TYPES_H_
#define ENCLAVE_MYENCLAVE_TRUSTED_INCLUDE_TYPES_H_

#include <sys/types.h>
#include <stdint.h>


#ifdef __cplusplus
extern "C" {
#endif

#ifdef __CHECKER__
#define OVS_BITWISE __attribute__((bitwise))
#define OVS_FORCE __attribute__((force))
#else
#define OVS_BITWISE
#define OVS_FORCE
#endif

/* The ovs_be<N> types indicate that an object is in big-endian, not
 * native-endian, byte order.  They are otherwise equivalent to uint<N>_t. */
typedef uint16_t OVS_BITWISE ovs_be16;
typedef uint32_t OVS_BITWISE ovs_be32;
typedef uint64_t OVS_BITWISE ovs_be64;

#endif /* ENCLAVE_MYENCLAVE_TRUSTED_INCLUDE_TYPES_H_ */
