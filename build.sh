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
./configure CFLAGS="-D SGX -I/home/jakob/test_ovs/OFTonSGX/untrusted" \
            LDFLAGS="-L/home/jakob/test_ovs/OFTonSGX/ovs/lib/ \
                     -L/home/jakob/test_ovs/OFTonSGX" \
            LIBS="-lsample"
make clean
make
make install
make modules_install
mkdir -p /usr/local/etc/openvswitch
ovsdb-tool create /usr/local/etc/openvswitch/conf.db vswitchd/vswitch.ovsschema
cd ..
cd ..
