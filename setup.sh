#!/bin/bash
git clone https://github.com/openvswitch/ovs.git
cd ovs
git checkout 53cc4b0
cd ..

cd TLSonSGX
git clone https://github.com/bl4ck5un/mbedtls-SGX.git
cd mbedtls-SGX/
git checkout 0ff0f8217f10a34754638a328fe02bd08c16e878
cd ..

git clone https://github.com/ARMmbed/mbedtls.git
cd mbedtls/
make
cd ../../

cp ./OFTonSGX/ovs_modified/ovs_sgx/*.c ./ovs/ofproto
cp ./OFTonSGX/ovs_modified/ovs_sgx/*.h ./ovs/ofproto
cp ./TLSonSGX/ovs_modified/ovs_sgx/stream-ssl.c ./ovs/lib
