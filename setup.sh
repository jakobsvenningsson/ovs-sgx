#!/bin/bash
git clone https://github.com/openvswitch/ovs.git
cd ovs
git checkout 53cc4b0
cp ../OFTonSGX/ovs_modified/ovs_sgx/*.c ./ofproto
