# Open vSwitch with SGX support

This repository contains a SGX enabled version of Open vSwitch with confidentiality and integrity guarantees fow flow tables and SSL context. This is done with the help of intel SGX enclaves.

##  How to run the project:

### Download dependencies
1. Install SGX
2. Run the setup script: `./setup.sh`. The setup script will download Open vSwitch and mbedtls, checkout the correct commits and replace the original source files in the OVS project with the modified source files for the SGX enabled OVS contained in `OFTonSGX/modified_sgx/ovs_sgx` and  `TLSonSGX/modified_sgx/ovs_sgx`.

### Compile
    1. Change the ENCLAVE_FILENAME constant in `TLSonSGX/untrusted/utils.h` and `OFTonSGX/untrusted/sgx-utils.c` to your OVS_PATH.
    2. Compile TLSonSGX, OFTonSGX and OVS by running the build script. The build script optionally takes a string of compilation flags as argument.
        The following flags are availible:
        * SGX - enables SGX protection of the OpenFlow flow tables.
        * SGX_TLS - enables SGX pretection of SSL.

For example, to compile the project with SGX enabled for both SSL and flow tables run `./build.sh "-D SGX -D SGX_TLS"`

### Run
Start OvS by executing the startup script: `./startup.sh`.

**If the project has been compiled with the `SGX_TLS flag, the ca-server (under ./CA_server) has to be running before running the startup script. Instructions on how to run and generate keys can be found in the README inside of the CA_server folder.**
