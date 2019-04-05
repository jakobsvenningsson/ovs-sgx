#!/bin/bash

source /opt/intel/sgxsdk/environment

# Clean up previous state
cd ..
rm /usr/local/etc/openvswitch/conf.db
pkill ovs
pkill CA_server
ovsdb-tool create /usr/local/etc/openvswitch/conf.db ovs/vswitchd/vswitch.ovsschema


cd ovs

echo $?
echo "Loading kernel module"
/sbin/modprobe openvswitch
echo $?
echo "Starting ovsdb-server"
ovsdb-server --remote=punix:/usr/local/var/run/openvswitch/db.sock \
    --remote=db:Open_vSwitch,Open_vSwitch,manager_options \
    --private-key=db:Open_vSwitch,SSL,private_key \
    --certificate=db:Open_vSwitch,SSL,certificate \
    --bootstrap-ca-cert=db:Open_vSwitch,SSL,ca_cert \
    --pidfile --detach --log-file

echo $?
ovs-vsctl --no-wait init
echo "Starting ovs-vswitch"
ovs-vswitchd --pidfile --log-file --detach 5>../benchmark/add_flow.log 

ovs-vsctl add-br br0

#ovs-vsctl set-controller br0 ssl:127.0.0.1:6633
#touch text.txt
#ovs-vsctl set-ssl text.txt text.txt text.txt
#ovs-vsctl set Bridge br0 protocols=OpenFlow10

cd ../benchmark

