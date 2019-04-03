# Open vSwitch with SGX support

This repository contains a library which provides the OpenFlow flow tables of Open vSwitch with confidentiality and integrity guarantees. This is done with the help of intel SGX enclaves.

##  How to compile the project:
1. Install SGX.
2. Run the setup script: `./setup.sh`. The setup script will download Open vSwitch using git, checkout the correct commit and replace the original source files with the modified source files for the SGX enabled OVS contained in `OFTonSGX/modified_sgx/ovs_sgx`.
3. Modify the absolute path in the `#TOKEN_FILENAME` and `#ENCLAVE_FILENAME` macros in `/OFTonSGX/untrusted/sgx-util.c` such that they point correctly to your `ovs` directory (which was created by the setup script in step 2).
4. Compile the project by running the build script: `./build.sh`.
