/*
 * inetinet.h
 *
 *  Created on: Apr 23, 2018
 *      Author: jorge
 */

#ifndef ENCLAVE_MYENCLAVE_TRUSTED_LIB_INETINET_H_
#define ENCLAVE_MYENCLAVE_TRUSTED_LIB_INETINET_H_

//#include <endian.h>

#ifndef __USE_KERNEL_IPV6_DEFS
/* IPv6 address */
struct in6_addr
  {
    union
      {
        uint8_t __u6_addr8[16];
#ifdef __USE_MISC
        uint16_t __u6_addr16[8];
        uint32_t __u6_addr32[4];
#endif
      } __in6_u;
#define s6_addr                 __in6_u.__u6_addr8
#ifdef __USE_MISC
# define s6_addr16              __in6_u.__u6_addr16
# define s6_addr32              __in6_u.__u6_addr32
#endif
  };
#endif /* !__USE_KERNEL_IPV6_DEFS */


#endif /* ENCLAVE_MYENCLAVE_TRUSTED_LIB_INETINET_H_ */
