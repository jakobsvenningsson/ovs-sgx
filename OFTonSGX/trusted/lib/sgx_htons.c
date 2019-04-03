/*
 * sgx_htons.c
 *
 *  Created on: May 7, 2018
 *      Author: jorge
 */

#include "sgx_htons.h"

uint16_t
htons(uint16_t x)
{
#if BYTE_ORDER == BIG_ENDIAN
  return x;
#elif BYTE_ORDER == LITTLE_ENDIAN
  return __bswap_16_sgx (x);
#else
# error "What kind of system is this?"
#endif
}

uint16_t
ntohs(uint16_t x)
{
#if BYTE_ORDER == BIG_ENDIAN
  return x;
#elif BYTE_ORDER == LITTLE_ENDIAN
  return __bswap_16_sgx (x);
#else
# error "What kind of system is this?"
#endif
}






uint32_t
htonl (uint32_t x)
{
#if BYTE_ORDER == BIG_ENDIAN
  return x;
#elif BYTE_ORDER == LITTLE_ENDIAN
  return __bswap_32_sgx (x);
#else
# error "What kind of system is this?"
#endif
}

uint32_t
ntohl(uint32_t x){
#if BYTE_ORDER == BIG_ENDIAN
  return x;
#elif BYTE_ORDER == LITTLE_ENDIAN
  return __bswap_32_sgx (x);
#else
# error "What kind of system is this?"
#endif
}
