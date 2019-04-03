/*
 * openflow-common.h
 *
 *  Created on: Apr 23, 2018
 *      Author: jorge
 */

#ifndef ENCLAVE_MYENCLAVE_TRUSTED_INCLUDE_OPENFLOW_COMMON_H_
#define ENCLAVE_MYENCLAVE_TRUSTED_INCLUDE_OPENFLOW_COMMON_H_

/* Version number:
 * Non-experimental versions released: 0x01 0x02
 * Experimental versions released: 0x81 -- 0x99
 */
/* The most significant bit being set in the version field indicates an
 * experimental OpenFlow version.
 */
enum ofp_version {
    OFP10_VERSION = 0x01,
    OFP11_VERSION = 0x02,
    OFP12_VERSION = 0x03,
    OFP13_VERSION = 0x04,
    OFP14_VERSION = 0x05,
    OFP15_VERSION = 0x06
};

#endif /* ENCLAVE_MYENCLAVE_TRUSTED_INCLUDE_OPENFLOW_COMMON_H_ */
