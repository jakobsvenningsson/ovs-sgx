
cd ovs
./boot.sh
./configure CFLAGS="-D SGX \
                    -I/home/jakob/SGX_Code/TLS_Project/TLSonSGX/SSL_Wrapper_SGX/mbedtls/include \
                    -I/home/jakob/link_test/include \
                    -I/home/jakob/SGX_Code/TLS_Project/TLSonSGX/SSL_Wrapper_SGX/App" \
            LDFLAGS="-L/home/jakob/SGX_Code/TLS_Project/TLSonSGX/OVS_SGX/ovs/lib \
                     -L/home/jakob/SGX_Code/TLS_Project/TLSonSGX/SSL_Wrapper_SGX/App \
                     -L/home/jakob/link_test/lib \
                     -L/home/jakob/SGX_Code/TLS_Project/TLSonSGX/SSL_Wrapper_SGX/mbedtls/library \
                     -lwrapper_sgx -lfoo -lmbedtls -lmbedx509 -lmbedcrypto -lstdc++"
make
make install
make modules_install

setfattr -n security.SMACK64 -v M /usr/local/sbin/ovs-vswitchd
mkdir -p /usr/local/etc/openvswitch
ovsdb-tool create /usr/local/etc/openvswitch/conf.db vswitchd/vswitch.ovsschema
cd ..
#-I/home/jakob/SGX_Code/TLS_Project/TLSonSGX/SSL_Wrapper_SGX/mbedtls-SGX/example/linux/App
#-I/home/jakob/SGX_Code/TLS_Project/TLSonSGX/SSL_Wrapper_SGX/App
#-L/home/jakob/SGX_Code/TLS_Project/TLSonSGX/SSL_Wrapper_SGX/mbedtls-SGX/example/linux/App\
