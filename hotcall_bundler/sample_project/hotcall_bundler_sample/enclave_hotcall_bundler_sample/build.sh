#!/bin/bash

source /opt/intel/sgxsdk/environment

make -C ../../../trusted clean
make -C ../../../trusted 

make -C ../../../untrusted clean
make -C ../../../untrusted 

make clean
make
