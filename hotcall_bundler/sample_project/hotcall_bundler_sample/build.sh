#!/bin/bash

source /opt/intel/sgxsdk/environment

make -C ../../src/trusted clean
make -C ../../src/trusted 

make -C ../../src/untrusted clean
make -C ../../src/untrusted 

make clean
make 
