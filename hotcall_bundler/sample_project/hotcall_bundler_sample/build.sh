#!/bin/bash

source /opt/intel/sgxsdk/environment

make -C ../../src/trusted -j4 clean
make -C ../../src/trusted

make -C ../../src/untrusted -j4 clean
make -C ../../src/untrusted -j4 

make clean
make 
