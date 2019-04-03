#!/bin/bash
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
./configure CFLAGS="-D SGX -I$PWD/OFTonSGX/untrusted" \
            LDFLAGS="-L$PWD/OFTonSGX/ovs/lib/ \
                     -L$PWD/OFTonSGX" \
            LIBS="-lsample"
make clean
make
make install
make modules_install
mkdir -p /usr/local/etc/openvswitch
ovsdb-tool create /usr/local/etc/openvswitch/conf.db vswitchd/vswitch.ovsschema
cd ..
