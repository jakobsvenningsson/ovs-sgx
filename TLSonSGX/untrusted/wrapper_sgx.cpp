#include <stdio.h>
#include <stdlib.h>
#include "mbedtls/x509_crt.h"
#include "mbedtls/x509.h"
#include <sgx_urts.h>

#include "enclave_u.h"
#include "utils.h"
#include "wrapper_ssl.h"


static sgx_enclave_id_t eid = 0;
static int enclave_status   = 10;

// #define PRINT_FUNC() printf("#####%s\n",__PRETTY_FUNCTION__)
#define PRINT_FUNC()

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;

string
getIMAmeasure(){
    string col1;
    string col2;
    string col3;
    string col4;
    string col5;
    string fileToMeasure = "/usr/local/sbin/ovs-vswitchd";
    string currentMeasure;

    ifstream inputFile("/sys/kernel/security/ima/ascii_runtime_measurements");
    string line;

    while (getline(inputFile, line)) {
        istringstream ss(line);
        ss >> col1 >> col2 >> col3 >> col4 >> col5;
        if (col5 == fileToMeasure) {
            currentMeasure = col4;
        }
    }
    currentMeasure = "sha256:TEST_MEASSUREMENT";

    string header     = "IMA";
    string withHeader = header + currentMeasure;
    return withHeader;
}

/* Library initialization */


int
SSL_library_init(){
    printf("Inside TLS ENCLAVE.\n");

    PRINT_FUNC();
    if (enclave_status == 10) {
        initialize_enclave(&eid);
        enclave_status = 0;
        printf("@@@SSL Library_init: enclave initilizaed\n");
    }
    string lastMeasureString = getIMAmeasure();
    float key_gen_time       = 0;
    const char * lastMeasure = lastMeasureString.c_str();
    ecall_ssl_library_init(eid, lastMeasure, 256);
    return 1;
}

/* CTX functions */
SSL_METHOD *
SSLv23_client_method(){
    PRINT_FUNC();
    static SSL_METHOD SSLv23_client = { MBEDTLS_SSL_IS_CLIENT,
                                        MBEDTLS_SSL_MAJOR_VERSION_3,
                                        MBEDTLS_SSL_MINOR_VERSION_0 };
    return &SSLv23_client;
}

SSL_CTX *
SSL_CTX_new(SSL_METHOD * ssl_method){
    PRINT_FUNC();
    SSL_CTX * ctx = (SSL_CTX *) calloc(1, sizeof(*ctx));
    ecall_ssl_ctx_new(eid);
    return ctx;
}

void
SSL_CTX_free(SSL_CTX * ctx){
    PRINT_FUNC();
    ecall_ssl_ctx_free(eid);
    free(ctx);
}

void
SSL_CTX_set_verify(SSL_CTX * ctx, int mode, void * reserved){
    // SSL_VERIFY_REQUIRED is hardcoded
    PRINT_FUNC();
    ecall_ssl_ctx_set_verify(eid);
}

void
SSL_set_verify(SSL * ssl, int mode, void * reserved){
    PRINT_FUNC();
}

long
SSL_CTX_set_mode(SSL_CTX * ctx, long mode){
    (void) ctx;

    /*
     * PolarSSL required to recall ssl_write with the SAME parameters
     * in case of WANT_WRITE and SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER
     * doesn't support directly
     * but since PolarSSL stores sent data as OFFSET (not pointer)
     * it is not a problem to move buffer (if data remains the same)
     * Cannot return error from SSL_CTX_set_mode
     * As result - do nothing
     */

    /* Open vSwitch sets SSL_MODE_ENABLE_PARTIAL_WRITE and SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER
     *  As mbedtls documentation "ssl write will do partial writes sometimes" probably ssl_write might need to be recalled based on return value"
     */
    PRINT_FUNC();
    return mode;
}

X509 *
SSL_get_peer_certificate(SSL * ssl){
    PRINT_FUNC();
    X509 * cert = (X509 *) calloc(1, sizeof(*cert));
    mbedtls_x509_crt_init(cert);
    ecall_ssl_get_peer_certificate(eid, cert);
    return cert;
}

X509_NAME *
X509_get_subject_name(X509 * cert){
    PRINT_FUNC();
    if (cert == NULL) {
        printf("@@@X509_get_subject_name:cert is Null\n");
    }
    return &(cert->subject);
}

char *
X509_NAME_oneline(X509_NAME * subject, char * buf, int size){
    PRINT_FUNC();
    char * name = (char *) calloc(1, 2048);
    mbedtls_x509_dn_gets(name, 2048, subject);
    return name;
}

void
SSL_load_error_strings(){
    PRINT_FUNC();
    ecall_ssl_load_error_strings(eid);
}

void
X509_free(X509 * crt){
    PRINT_FUNC();
    free(crt);
}

/* SSL functions */

SSL *
SSL_new(SSL_CTX * ctx){
    PRINT_FUNC();
    SSL * ssl = (SSL *) calloc(1, sizeof(*ssl));
    ecall_ssl_new(eid);
    printf("@@@SSL_new: After ecall\n");
    return ssl;
}

void
SSL_free(SSL * ssl){
    PRINT_FUNC();
    ecall_ssl_free(eid);
    free(ssl);
}

/*
 * PolarSSL functions return error code direcly,
 * it stores in ssl::last_error because some OpenSSL functions
 * assumes 0/1 retval only.
 */
int
SSL_get_error(const SSL * ssl, int ret){
    PRINT_FUNC();
    int ret1 = 0;
    ecall_ssl_get_error(eid, &ret1, ret);

    return ret1;
}

int
SSL_want(const SSL * ssl){
    PRINT_FUNC();
    int ret1 = 0;
    ecall_ssl_get_error(eid, &ret1, 1);
    if (ret1 == SSL_ERROR_WANT_READ) {
        return SSL_READING;
    } else if (ret1 == SSL_ERROR_WANT_WRITE)     {
        return SSL_WRITING;
    }
    return ret1;
}

int
SSL_set_fd(SSL * ssl, int fd){
    PRINT_FUNC();
    int ret = 0;
    ecall_ssl_set_fd(eid, &ret, fd);
    return 1;
}

/* PolarSSL read/write functions work as OpenSSL analogues */
int
SSL_read(SSL * ssl, void * buf, int num){
    PRINT_FUNC();
    int ret = 0;
    ecall_ssl_read(eid, &ret, (char *) buf, num);
    return ret;
}

int
SSL_write(SSL * ssl, const void * buf, int num){
    PRINT_FUNC();
    int ret = 0;
    ecall_ssl_write(eid, &ret, (char *) buf, num);
    return ret;
}

int
SSL_connect(SSL * ssl){
    PRINT_FUNC();
    int ret = 0;
    ecall_ssl_connect(eid, &ret);
    // printf("@@@SSL_connect:after ecall return value is %d\n", ret);
    return ret;
}

int
SSL_accept(SSL * ssl){
    PRINT_FUNC();
    int ret = 0;
    ecall_ssl_accept(eid, &ret);
    return ret;
}

int
SSL_get_verify_mode(SSL * ssl){
    PRINT_FUNC();
    return 1;
}

int
SSL_get_state(SSL * ssl){
    PRINT_FUNC();
    int ret = 0;
    ecall_ssl_get_state(eid, &ret);
    return ret;
}

// Following methods are not needed in SGX based ovs stream-ssl.c
int
SSL_CTX_set_cipher_list(SSL_CTX * ctx, const char * str){
    PRINT_FUNC();
    return 1;
}

void
SSL_set_msg_callback(SSL_CTX * ctx, void * reserved_for_cb){
    PRINT_FUNC();
    // printf("@@@SSL_set_msg_callback:method is not implemented\n");
}

void
SSL_set_msg_callback_arg(SSL_CTX * ctx, void * arg){
    PRINT_FUNC();
    // printf("@@@SSL_set_msg_callback_arg: method is not implemented\n");
}

void
SSL_CTX_set_tmp_dh_callback(SSL_CTX * ctx, void * reserved_for_cb){
    PRINT_FUNC();
    // printf("@@@SSL_CTX_set_tmp_dh_callback:method is not implemented\n");
}

long
SSL_CTX_set_session_cache_mode(SSL_CTX * ctx, long mode){
    PRINT_FUNC();
    // in mbedtls if mbedtls_ssl_conf_session_cache not set then no session resuming is done
    return 1;
}

long
SSL_CTX_set_options(SSL_CTX * ctx, long options){
    PRINT_FUNC();
    // In mbedtls SSLv2 never implemented and SSLv3 is disabled by default
    // Open vSwitch sets the options SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3
    return options;
}

int
SSL_CTX_check_private_key(const SSL_CTX * ctx){
    PRINT_FUNC();
    // Private key and certificate are within the enclave and would never leave the enclave
    // printf("@@@SSL_CTX_check_private_key:Private key and certificate are within the enclave and would never leave the enclave\n");
    return 1;
}

int
SSL_shutdown(SSL * ssl){
    PRINT_FUNC();
    int ret = 0;
    ecall_ssl_shutdown(eid, &ret);
    return ret;
}
