#!/bin/bash

ROOT_FOLDER=$PWD
FLAGS=$1
source /opt/intel/sgxsdk/environment

echo "%%%%%%%%%%%%%%%%%%%%%%%%% BUILDING OFTonSGX"
make -C OFTonSGX/ clean
make -C OFTonSGX/ SGX_MODE=HW SGX_PRERELEASE=1 SGX_DEBUG=0 LFLAGS="$FLAGS"
cp OFTonSGX/enclave.signed.so ./ovs

echo "%%%%%%%%%%%%%%%%%%%%%%%%% BUILDING TLSonSGX"
make -C TLSonSGX/ clean
make -C TLSonSGX/ SGX_MODE=HW SGX_PRERELEASE=1 SGX_DEBUG=0 LFLAGS="$FLAGS"
make -C TLSonSGX/ wrapper_library
cp TLSonSGX/tls_enclave.signed.so ./ovs

echo "%%%%%%%%%%%%%%%%%%%%%%%%% BUILDING OvS"
cd ovs
./boot.sh
./configure	CFLAGS="$FLAGS -I${ROOT_FOLDER}/TLSonSGX/untrusted -I${ROOT_FOLDER}/TLSonSGX/App  -I${ROOT_FOLDER}/OFTonSGX/untrusted -I${ROOT_FOLDER}/benchmark/include -I${ROOT_FOLDER}/TLSonSGX/mbedtls/include" \
            	LDFLAGS="-L$ROOT_FOLDER/ovs/lib/ -L$ROOT_FOLDER/OFTonSGX -L${ROOT_FOLDER}/TLSonSGX/mbedtls/library -L$ROOT_FOLDER/TLSonSGX" \
            	LIBS="-lOFTonSGX -lTLSonSGX -lmbedtls -lmbedx509 -lmbedcrypto -lpthread -lstdc++"


make clean
make -j4
echo "Compilation successfull."

make install
make modules_install

cd ..

setfattr -n security.SMACK64 -v M /usr/local/sbin/ovs-vswitchd
mkdir -p /usr/local/etc/openvswitch
ovsdb-tool create /usr/local/etc/openvswitch/conf.db ovs/vswitchd/vswitch.ovsschema
