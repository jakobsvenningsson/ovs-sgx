#!/bin/bash
git clone https://github.com/openvswitch/ovs.git
git -C ovs/ checkout 53cc4b0

git -C TLSonSGX/ clone https://github.com/ARMmbed/mbedtls.git
make -C TLSonSGX/mbedtls

git -C TLSonSGX/ clone https://github.com/bl4ck5un/mbedtls-SGX.git
git -C TLSonSGX/mbedtls-SGX/ checkout 0ff0f8217f10a34754638a328fe02bd08c16e878
make -C TLSonSGX/mbedtls-SGX/

cp ./OFTonSGX/ovs_modified/ovs_sgx/*.c ./ovs/ofproto
cp ./OFTonSGX/ovs_modified/ovs_sgx/*.h ./ovs/ofproto
cp ./TLSonSGX/ovs_modified/ovs_sgx/stream-ssl.c ./ovs/lib
