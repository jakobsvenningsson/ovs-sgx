#!/bin/bash
ROOT_FOLDER=$PWD
source /opt/intel/sgxsdk/environment
echo "%%%%%%%%%%%%%%%%%%%%%%%%% BUILDING OFTonSGX"
cd OFTonSGX
make clean
make SGX_MODE=HW SGX_PRERELEASE=1 SGX_DEBUG=0 LFLAGS="-D HOTCALL -D TIMEOUT"
cp enclave.signed.so ../ovs
cd ..

echo "%%%%%%%%%%%%%%%%%%%%%%%%% BUILDING OvS"
cd ovs
./boot.sh
./configure CFLAGS="-D SGX -I${ROOT_FOLDER}/OFTonSGX/untrusted" \
            	LDFLAGS="-L$ROOT_FOLDER/ovs/lib/ \
                     	 -L$ROOT_FOLDER/OFTonSGX" \
            	LIBS="-lOFTonSGX"
make clean
make
make install
make modules_install
mkdir -p /usr/local/etc/openvswitch
ovsdb-tool create /usr/local/etc/openvswitch/conf.db vswitchd/vswitch.ovsschema
cd ..
