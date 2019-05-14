#!/bin/bash
source /opt/intel/sgxsdk/environment

rm /usr/local/etc/openvswitch/conf.db
pkill ovs
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
#ovs-vswitchd  --pidfile 
#sudo cset shield -e ovs-vswitchd -- --pidfile --log-file=$HOME/ovs-sgx/log 5>&1 
#sudo chrt -f 99 taskset -c 2,3 ovs-vswitchd --pidfile --log-file=$HOME/ovs-sgx/log 5>/home/jakob/ovs-sgx/$1 #--log-file=$HOME/ovs-log

#cset shield --exec ovs-vswitchd -- --detach --pidfile --log-file=$home/ovs-sgx/log 5>/home/jakob/ovs-sgx/$1 
ovs-vswitchd --pidfile
#taskset -c 2,3 ovs-vswitchd --pidfile --log-file=$home/ovs-sgx/log 5>/home/jakob/ovs-sgx/$1 
#sudo chrt -f 99 taskset -c 1,2,3 ovs-vswitchd --pidfile --log-file=$home/ovs-sgx/log #5>&1 #/home/jakob/ovs-sgx/$1 
#sudo taskset -c 2,3 chrt -r 1 ovs-vswitchd --pidfile --log-file=$home/ovs-sgx/log 5>/home/jakob/ovs-sgx/$1 
ovs-vsctl add-br br0
#ovs-vsctl set-controller br0 ssl:127.0.0.1:6633
#touch text.txt
#ovs-vsctl set-ssl text.txt text.txt text.txt
#ovs-vsctl set Bridge br0 protocols=OpenFlow10

cd ..

