/*
 * random.h
 *
 *  Created on: May 7, 2018
 *      Author: jorge
 */

#ifndef ENCLAVE_MYENCLAVE_TRUSTED_LIB_SGX_HTONS_H_
#define ENCLAVE_MYENCLAVE_TRUSTED_LIB_SGX_HTONS_H_

#include <endian.h>
#include <stdint.h>

/* Macro to test version of GCC.  Returns 0 for non-GCC or too old GCC. */
#ifndef __GNUC_PREREQ
# if defined __GNUC__ && defined __GNUC_MINOR__
#  define __GNUC_PREREQ(maj, min) \
	((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
# else
#  define __GNUC_PREREQ(maj, min) 0
# endif
#endif /* __GNUC_PREREQ */

/* Swap bytes in 16-bit value.  */
#define __bswap_constant_16_sgx(x)                                        \
  ((__uint16_t) ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8)))

static inline unsigned short __builtin_bswap16(unsigned short a)
{
  return (a<<8)|(a>>8);
}
static __inline __uint16_t __bswap_16_sgx (__uint16_t __bsx)
{
#if __GNUC_PREREQ (4, 8)
  return __builtin_bswap16 (__bsx);

#else
  return __bswap_constant_16_sgx (__bsx);
#endif
}

/* Swap bytes in 32-bit value.  */
#define __bswap_constant_32_sgx(x)                                        \
  ((((x) & 0xff000000u) >> 24) | (((x) & 0x00ff0000u) >> 8)        \
   | (((x) & 0x0000ff00u) << 8) | (((x) & 0x000000ffu) << 24))
static __inline __uint32_t
__bswap_32_sgx (__uint32_t __bsx)
{
#if __GNUC_PREREQ (4, 3)
  return __builtin_bswap32 (__bsx);
#else
  return __bswap_constant_32_sgx (__bsx);
#endif
}

uint32_t htonl (uint32_t x);
uint32_t ntohl (uint32_t x);
uint16_t ntohs(uint16_t x);
uint16_t htons(uint16_t x);

#endif /* ENCLAVE_MYENCLAVE_TRUSTED_LIB_SGX_HTONS_H_ */
