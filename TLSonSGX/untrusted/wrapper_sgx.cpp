#include <stdio.h>
#include <stdlib.h>
#include "mbedtls/x509_crt.h"
#include "mbedtls/x509.h"
#include <sgx_urts.h>
#include <pthread.h>

#include "spinlock.h"
#include "common.h"
#include "hotcall-producer.h"

#include "enclave_u.h"
#include "utils.h"
#include "wrapper_ssl.h"

static sgx_enclave_id_t eid = 0;
static int enclave_status   = 10;

// #define PRINT_FUNC() printf("#####%s\n",__PRETTY_FUNCTION__)
#define PRINT_FUNC()

#ifdef HOTCALL_TLS
# define ECALL(f, has_return, n_args, args ...) \
    argument_list arg_list; \
    void * return_val; \
    compile_arg_list(&return_val, &arg_list, has_return, n_args, args); \
    make_hotcall(&ctx, hotcall_ ## f, &arg_list, return_val)
    # define CAST(X) &X
#else
# define ECALL(f, has_return, n_args, args ...) \
    f(eid, args)
# define CAST(X) X
#endif /* ifdef HOTCALL */

void *
ecall_polling_thread(void * vargp){
    printf("Running polling thread.\n");
    int ecall_return;
    MAKE_ECALL_ARGS(start_poller_tls, &ecall_return, eid, &ctx);
    if (ecall_return == 0) {
        printf("Application ran with success\n");
    } else {
        printf("Application failed %d \n", ecall_return);
    }
}

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

static int nr_init = 0;

int
SSL_library_init(){
    printf("Inside TLS ENCLAVE %d.\n", nr_init++);

    PRINT_FUNC();
    if (enclave_status == 10) {
        initialize_enclave(&eid);
        enclave_status = 0;
        printf("@@@SSL Library_init: enclave initilizaed\n");

        #ifdef HOTCALL_TLS
        printf("TLS HOTCALLS ENABLED STARTING THREAD.\n");
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, ecall_polling_thread, NULL);
        #else
        puts("NO HOTCALLS.");
        #endif
        #ifdef TIMEOUT_TLS
        puts("TIMEOUT ENABLED\n");
        #endif
    }
    string lastMeasureString = getIMAmeasure();
    float key_gen_time       = 0;
    const char * lastMeasure = lastMeasureString.c_str();
    printf("Before ecall_ssl_library_init\n");
    int mSize = 256;
    ECALL(ecall_ssl_library_init, false, 2, lastMeasure, CAST(mSize));
    printf("After ecall_ssl_library_init\n");
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
    SSL_CTX * ssl_ctx = (SSL_CTX *) calloc(1, sizeof(*ssl_ctx));
    ECALL(ecall_ssl_ctx_new, false, 0, NULL);
    return ssl_ctx;
}

void
SSL_CTX_free(SSL_CTX * ssl_ctx){
    PRINT_FUNC();
    ECALL(ecall_ssl_ctx_free, false, 0, NULL);
    free(ssl_ctx);
}

void
SSL_CTX_set_verify(SSL_CTX * ssl_ctx, int mode, void * reserved){
    // SSL_VERIFY_REQUIRED is hardcoded
    PRINT_FUNC();
    ECALL(ecall_ssl_ctx_set_verify, false, 0, NULL);
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
    ECALL(ecall_ssl_get_peer_certificate, false, 1, cert);
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
    ECALL(ecall_ssl_load_error_strings, false, 0, NULL);
}

void
X509_free(X509 * crt){
    PRINT_FUNC();
    free(crt);
}

/* SSL functions */

SSL *
SSL_new(SSL_CTX * ssl_ctx){
    PRINT_FUNC();
    SSL * ssl = (SSL *) calloc(1, sizeof(*ssl));
    ECALL(ecall_ssl_new, false, 0, NULL);
    printf("@@@SSL_new: After ecall\n");
    return ssl;
}

void
SSL_free(SSL * ssl){
    PRINT_FUNC();
    ECALL(ecall_ssl_free, false, 0, NULL);
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
    ECALL(ecall_ssl_get_error, true, 1, &ret1, CAST(ret));
    return ret1;
}

int
SSL_want(const SSL * ssl){
    PRINT_FUNC();
    int ret1 = 0;
    int dummy = 1;
    ECALL(ecall_ssl_get_error, true, 1, &ret1, CAST(dummy));
    if (ret1 == SSL_ERROR_WANT_READ) {
        return SSL_READING;
    } else if (ret1 == SSL_ERROR_WANT_WRITE) {
        return SSL_WRITING;
    }
    return ret1;
}

int
SSL_set_fd(SSL * ssl, int fd){
    PRINT_FUNC();
    int ret = 0;
    ECALL(ecall_ssl_set_fd, true, 1, &ret, CAST(fd));
    return 1;
}

/* PolarSSL read/write functions work as OpenSSL analogues */
int
SSL_read(SSL * ssl, void * buf, int num){
    PRINT_FUNC();
    int ret = 0;
    ECALL(ecall_ssl_read, true, 2, &ret, (char *) buf, CAST(num));
    return ret;
}

int
SSL_write(SSL * ssl, const void * buf, int num){
    PRINT_FUNC();
    int ret = 0;
    ECALL(ecall_ssl_write, true, 2, &ret, (char *) buf, CAST(num));
    return ret;
}

int
SSL_connect(SSL * ssl){
    PRINT_FUNC();
    int ret = 0;
    ECALL(ecall_ssl_connect, true, 0, &ret, NULL);
    return ret;
}

int
SSL_accept(SSL * ssl){
    PRINT_FUNC();
    int ret = 0;
    ECALL(ecall_ssl_accept, true, 0, &ret, NULL);
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
    ECALL(ecall_ssl_get_state, true, 0, &ret, NULL);
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
}

void
SSL_set_msg_callback_arg(SSL_CTX * ctx, void * arg){
    PRINT_FUNC();
}

void
SSL_CTX_set_tmp_dh_callback(SSL_CTX * ctx, void * reserved_for_cb){
    PRINT_FUNC();
}

long
SSL_CTX_set_session_cache_mode(SSL_CTX * ctx, long mode){
    PRINT_FUNC();
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
    return 1;
}

int
SSL_shutdown(SSL * ssl){
    PRINT_FUNC();
    int ret = 0;
    ECALL(ecall_ssl_shutdown, true, 0, &ret, NULL);
    return ret;
}
