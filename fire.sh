#!/bin/bash
N_FLOWS=$2
ITERS=3
cset shield --cpu 2,3
cset shield --kthread on
for n in `seq 0 $ITERS`; do
  #sudo ./startup.sh $1
  sudo ./startup.sh "${1}_${n}"
  sleep 1
  for i in `seq 0 $N_FLOWS`; do
    #n=$(($i%100))
    #if [ n == 0 ]; then
    #  sleep 1
    #fi
    ovs-ofctl add-flow br0 "in_port=1,priority=$i,actions=drop"
    #ovs-ofctl dump-flows br0 > /dev/null
    #ovs-ofctl del-flows br0 "in_port=1,priority=$i" --strict
    #sleep 0.3
  done
  sudo pkill -9 ovs
done

cset shield --reset

STR=""
for n in `seq 0 $ITERS`; do
  STR+="${1}_${n} "
done

cat $STR > $1
