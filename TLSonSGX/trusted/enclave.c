#include "enclave_t.h"
#include "sgx_trts.h"
#include "sgx_tseal.h"
#include "sgx_tprotected_fs.h"

#include "mbedtls/entropy.h"
#include "mbedtls/md5.h"
#include "mbedtls/ssl.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/certs.h"
#include "mbedtls/x509.h"
#include "mbedtls/x509_csr.h"
#include "mbedtls/compat-1.3.h"
#include "mbedtls/net_v.h"
#include "mbedtls/net_f.h"
#include "mbedtls/error.h"
#include "mbedtls/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "enclave.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#include "hotcall-trusted.h"
#include "enclave-utils.h"
#include "sealing/seal-ssl-resource.h"




#define SSL_ERROR_WANT_READ POLARSSL_ERR_NET_WANT_READ
#define SSL_ERROR_WANT_WRITE POLARSSL_ERR_NET_WANT_WRITE
#define SSL_AD_CLOSE_NOTIFY POLARSSL_ERR_SSL_PEER_CLOSE_NOTIFY

#define X509 x509_crt
#define X509_NAME mbedtls_x509_name


#define SSL_VERIFY_PEER 1
#define SSL_VERIFY_FAIL_IF_NO_PEER_CERT 2

/*
 * SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER is not used at the moment.
 */
#define SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER 1

//CA Server
#define SERVER_PORT "4433"
#define SERVER_NAME "127.0.0.1"

static SSL *ssl;
static SSL_CTX *ctx;
static mbedtls_x509_crt *cacert;
static mbedtls_x509_crt *cert;
static mbedtls_pk_context key;
static bool keys_generated=false;

static mbedtls_entropy_context g_entropy_context;
static mbedtls_ctr_drbg_context g_ctr_drbg_context;

size_t
send_recv(mbedtls_ssl_context ssl, char* send_buf, char recv_buf1[6000]) {
        int write_res = 10;
        int temp = 1;
        write_res = mbedtls_ssl_write( &ssl, send_buf, 2000 );
        if (write_res > 0) {
                ocall_print_string(&temp,"####Enclave:send_recv:write result is successfull\n");
        } else {
                ocall_print_string(&temp,"###Enclave:send_recv:write has failed");
        }

        size_t len=0;
        int ret;
        do {
                len = 6000;
                memset(recv_buf1, 0, sizeof(recv_buf1));
                ret = mbedtls_ssl_read(&ssl, recv_buf1, len);
                if(ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE)
                        continue;
                if( ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY )
                        break;
                if( ret < 0 ) {
                        ocall_print_string(&temp,"###Enclave:send_recv:failed\n  ! mbedtls_ssl_read returned");
                        break;
                }
                if( ret == 0 ) {
                        ocall_print_string(&temp,"###Enclave:send_recv:\n\nEOF\n\n");
                        break;
                }
                len = ret;
                return ret;
        }
        while(1);
}

void
load_sealed_ssl_resources(SSL_resource *rs) {
    for(size_t i = 0; i < N_SSL_RESOURCES; ++i) {
        uint32_t sealed_len = rs[i].len;
        uint8_t sealed_data[sealed_len];
        memcpy(sealed_data, rs[i].sealed_data, sealed_len);
        switch(rs[i].type) {
            case SSL_RESOURCE_CA_CERT:
                cacert = unseal_certificate(sealed_data, sealed_len);
                break;
            case SSL_RESOURCE_CERT:
                cert = unseal_certificate(sealed_data, sealed_len);
                break;
            case SSL_RESOURCE_PK:
                unseal_key(sealed_data, sealed_len, &key);
                break;
            default:
                printf("Unknown ssl resource %d.\n", rs[i].type);
                assert(false);
        }
    }
}

void
generate_keys(const char *currMeasure, SSL_resource *rs, bool *write_resources_to_disk) {

        mbedtls_pk_init(&key);


        // Check if sealed ssl context exist.
        if(all_ssl_resources_exist(rs)) {
            // Load from file and exit
            load_sealed_ssl_resources(rs);
            return;
        }

        // No sealed context exist, request resources from CA authority.

        int ret;
        int temp=10;
        if((ret = mbedtls_pk_setup(&key, mbedtls_pk_info_from_type(MBEDTLS_PK_RSA))) != 0) {
                ocall_print_string(&temp,"###Enclave:generate_keys:failed\n  !  mbedtls_pk_setup returned");
                return;
        } else {
                ocall_print_string(&temp,"###Enclave:generate_keys:mbedtls_pk_setup success\n");
        }


        size_t pk_length = 2048;
        ret = mbedtls_rsa_gen_key( mbedtls_pk_rsa(key), mbedtls_ctr_drbg_random, &g_ctr_drbg_context, pk_length, 65537 );
        if(ret != 0) {
                ocall_print_string(&temp,"###Enclave:generate_keys:failed\n  !  mbedtls_rsa_gen_key");
                return;
        } else {
                ocall_print_string(&temp,"###Enclave:generate_keys:Keys generated successfully \n");
        }


        //Private Key
        unsigned char prk_buf[pk_length];
        memset(prk_buf, 0, pk_length);
        if((ret = mbedtls_pk_write_key_pem(&key, prk_buf, pk_length)) != 0) {
            printf("Failed to generate private key.\n");
        }

        sgx_status_t sgx_res;
        sgx_res = seal_ssl_resource(prk_buf, strlen(prk_buf), &rs[2]);
        if(sgx_res != SGX_SUCCESS) {
            printf("Failed to seal private key.\n");
        }
        rs[2].type = SSL_RESOURCE_PK;


        //Public Key
        unsigned char puk_buf[pk_length];
        memset(puk_buf, 0, pk_length);
        if((ret = mbedtls_pk_write_pubkey_pem(&key, puk_buf, pk_length)) != 0) {
            printf("Failed to generate public key.\n");
        }

        mbedtls_x509write_csr req;
        mbedtls_x509write_csr_init(&req);
        mbedtls_x509write_csr_set_md_alg(&req, MBEDTLS_MD_SHA256);
        if((ret = mbedtls_x509write_csr_set_subject_name(&req, "CN=openvswitch")) != 0) {
            printf("Failed to set subject name.\n");
        }
        mbedtls_x509write_csr_set_key(&req, &key);

        unsigned char csr_buf[pk_length];
        memset(csr_buf, 0, pk_length);
        if((ret = mbedtls_x509write_csr_pem(&req, csr_buf, pk_length, mbedtls_ctr_drbg_random, &g_ctr_drbg_context)) != 0) {
            printf("Failed to create csr request.\n");
        }

        mbedtls_net_context server_fd;
        mbedtls_ssl_context ssl;
        mbedtls_ssl_config conf;
        mbedtls_net_init( &server_fd );
        mbedtls_ssl_init( &ssl );
        mbedtls_ssl_config_init( &conf );
        mbedtls_net_connect( &server_fd, SERVER_NAME,SERVER_PORT, MBEDTLS_NET_PROTO_TCP );
        mbedtls_ssl_config_defaults( &conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT );
        mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_NONE );
        mbedtls_ssl_conf_rng( &conf, mbedtls_ctr_drbg_random, &g_ctr_drbg_context);
        mbedtls_ssl_setup( &ssl, &conf );
        mbedtls_ssl_set_bio( &ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL );

        int res = 10;
        res = mbedtls_ssl_handshake(&ssl);
        if(res != 0) {
                ocall_print_string(&temp,"###Enclave: handshake failed.l\n");
                return;
        }
        ocall_print_string(&temp,"###Enclave:generate_keys:Handshake with CA is successfull\n");
        size_t cer_len = 0;

        char ima_res[128];
        send_recv(ssl, (char *) currMeasure, ima_res);
        ocall_print_string(&temp,"###Enclave:generate_keys:IMA Measurement validation result by CA is ");
        ocall_print_string(&temp,ima_res);
        ocall_print_string(&temp,"____\n");

        if (strcmp(ima_res,"MOK") != 0) {
                ocall_print_string(&temp,"###Enclave:generate_keys:IMA measurement has changed, no certificate received from CA");
                return;
        }
        ocall_print_string(&temp,"###Enclave:generate_keys:IMA is OK\n");
        char cer_buf[6000];
        cer_len= send_recv(ssl, csr_buf, cer_buf);
        cert = malloc(2048);
        mbedtls_x509_crt_init(cert);

        // Find start index of certificate part of PEM file
        char word[] = "-----";
        char *start = strstr(cer_buf, word);
        int cert_length = cer_len - (start - cer_buf);
        char buf[32];
        ocall_print_string(&temp, buf);
        char *cer_buf1=(char*)malloc(cert_length + 1);
        cer_buf1[cert_length] = '\0';
        memcpy(cer_buf1, start, cert_length);
        res = 10;
        res = mbedtls_x509_crt_parse(cert, cer_buf1, cert_length + 1);
        if (res != 0) {
                ocall_print_string(&temp,"###Enclave:generate_keys:Switch certificate parsing failed\n");
                return;
        }
        ocall_print_string(&temp,"###Enclave:generate_keys:Switch certificate is parsed successfully\n");

        // Seal certificate
        sgx_res = seal_ssl_resource(cer_buf1, cert_length + 1, &rs[1]);
        if(sgx_res != SGX_SUCCESS) {
            printf("Failed to store certificate in sealed storage.\n");
        }
        rs[1].type = SSL_RESOURCE_CERT;

        cacert = ( mbedtls_x509_crt *) mbedtls_ssl_get_peer_cert(&ssl);
        if(cacert == NULL) {
            printf("Failed to get server certificate.\n");
        }

        // Seal CA certificate
        sgx_res = seal_ssl_resource(cacert->raw.p, cacert->raw.len, &rs[0]);
        if(sgx_res != SGX_SUCCESS) {
            printf("Failed to store ca certificate in sealed storage.\n");
        }
        rs[0].type = SSL_RESOURCE_CA_CERT;

        printf("sealing of ca_cert ok\n");

        *write_resources_to_disk = true;


        mbedtls_ssl_close_notify(&ssl);
        mbedtls_net_free(&server_fd);
        mbedtls_ssl_free(&ssl);
        mbedtls_ssl_config_free(&conf);
}

int
ecall_ssl_read(char *buf, size_t buf_len) {
        int ret = 1;
        ssl->last_error = mbedtls_ssl_read(&ssl->cntx, buf, buf_len);
        return ssl->last_error;
}

int
ecall_ssl_write(char *buf, size_t buf_len) {
        ssl->last_error = mbedtls_ssl_write(&ssl->cntx, buf, buf_len);
        return ssl->last_error;
}

int
ecall_ssl_get_error(int ret) {
        return ssl != NULL ? ssl->last_error : 100;
}

int
ecall_ssl_connect() {
        int ret=10;
        int res=10;
        ret = mbedtls_ssl_handshake(&ssl->cntx);
        if (ret == 0) {
                ret = 1;
        } else if (ret == MBEDTLS_ERR_SSL_WANT_READ) {
                //ocall_print_string(&res, "###Enclave: ecall_ssl_connect result is MBEDTLS_ERR_SSL_WANT_READ\n");
        } else if (ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
                //ocall_print_string(&res, "###Enclave: ecall_ssl_connect result is MBEDTLS_ERR_SSL_WANT_WRITE\n");
        } else if (ret == MBEDTLS_ERR_SSL_HELLO_VERIFY_REQUIRED) {
                //ocall_print_string(&res, "###Enclave: ecall_ssl_connect result is  MBEDTLS_ERR_SSL_HELLO_VERIFY_REQUIRED\n");
        }

        if (ssl != NULL) {
                ssl->last_error = ret;
                return ssl->last_error;
        } else {
                return 101;
        }
}

int
ecall_ssl_set_fd(int fd) {
        ssl->fd =fd;
        mbedtls_ssl_set_bio(&ssl->cntx, &ssl->fd, mbedtls_net_send, mbedtls_net_recv, NULL);
        ssl->last_error = 0;
        return 1;
}

int
ecall_ssl_accept() {
        ssl->last_error = mbedtls_ssl_handshake(&ssl->cntx);
        return ssl->last_error;
}

void
ecall_ssl_library_init(const char *ima_buf, size_t ima_buf_len, SSL_resource *r, bool *write_resources_to_disk) {
        mbedtls_entropy_init(&g_entropy_context);
        mbedtls_ctr_drbg_init(&g_ctr_drbg_context);
        int ret=0;
        int temp=10;
        ocall_print_string(&ret, "###Enclave:ecall_ssl_library_init\n");
        const char *pers = "ssl_client2";
        if((ret = mbedtls_ctr_drbg_seed( &g_ctr_drbg_context, mbedtls_entropy_func, &g_entropy_context,
                                         (const unsigned char *) pers,
                                         strlen(pers))) != 0) {
                ocall_print_string(&temp, " failed\n  ! mbedtls_ctr_drbg_seed returned \n");
                return;
        }

        ocall_print_string(&temp, "###Enclave: ecall_ssl_library_init: Seed was success\n");

        /* SSL_library_init() always returns "1" */
        if(!keys_generated) {
                ocall_print_string(&temp, "###Enclave: ecall_ssl_library_init: Generating Keys\n");
                generate_keys(ima_buf, r, write_resources_to_disk);
                keys_generated=true;
        } else {
                ocall_print_string(&temp, "###Enclave: ecall_ssl_library_init: Keys are already generated\n");
        }

        return;
}

void
ecall_ssl_load_error_strings() {
        return;
}

void
ecall_ssl_new() {
        int res;
        int ret = 0;
        ssl = (SSL*)calloc(1, sizeof(*ssl));
        mbedtls_ssl_init(&ssl->cntx);
        mbedtls_ssl_config *l_conf=(mbedtls_ssl_config *)calloc(1,sizeof(mbedtls_ssl_config));
        mbedtls_ssl_config_init(l_conf);
        mbedtls_ssl_config_defaults( l_conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT );
        ssl->cntx.conf=l_conf;
        mbedtls_ssl_conf_authmode((mbedtls_ssl_config *) ssl->cntx.conf, ctx->authmode);
        mbedtls_ssl_conf_ca_chain((mbedtls_ssl_config *) ssl->cntx.conf, &ctx->CA_cert, NULL);
        mbedtls_ssl_conf_rng((mbedtls_ssl_config *) ssl->cntx.conf, ctr_drbg_random, &g_ctr_drbg_context );
        res = mbedtls_ssl_conf_own_cert((mbedtls_ssl_config *) ssl->cntx.conf, &ctx->cert, &ctx->pk);
        if (res != 0) {
                ocall_print_string(&ret, "###Enclave:ecall_ssl_new:key and cert load fail\n");
        }
        ocall_print_string(&ret, "###Enclave:ecall_ssl_new:key and cert loaded success\n");

        if((res = mbedtls_ssl_setup(ssl, ssl->cntx.conf)) != 0) {
                ocall_print_string( &ret, " failed\n  ! mbedtls_ssl_setup returned \n\n");
                return;
        }

        ocall_print_string(&ret, "###Enclave:ecall_ssl_new:: ssl setup success\n");

        ssl->fd = -1;
        ssl->ssl_ctx = ctx;
}

void
ecall_ssl_free() {
        int ret =0;
        ssl->cntx.session=NULL;
        free(ssl->cntx.session);
        ssl->cntx.conf=NULL;
        free((mbedtls_ssl_config *) ssl->cntx.conf);
        free(&ssl->cntx);
        ssl->ssl_ctx->ssl_method=NULL;
        free(ssl->ssl_ctx->ssl_method);
        ssl->ssl_ctx=NULL;
        free(ssl->ssl_ctx);
        ssl->ssl_ctx=NULL;
        ssl = NULL;
        free(ssl);
        ocall_print_string(&ret, "###Enclave:ecall_ssl_free:Exiting Enclave\n");
}

void
ecall_ssl_ctx_free() {
        ctx=NULL;
        free(ctx);
}

void
ecall_ssl_ctx_set_verify() {
        ctx->authmode =  MBEDTLS_SSL_VERIFY_REQUIRED | MBEDTLS_SSL_VERIFY_OPTIONAL;
        ctx->authmode =  MBEDTLS_SSL_VERIFY_OPTIONAL;
        // we initilize ssl->cntx.conf later in ssl_new
        //mbedtls_ssl_conf_authmode(ssl->cntx.conf, MBEDTLS_SSL_VERIFY_REQUIRED);
}

void
ecall_ssl_ctx_new() {
        /*hardcoded for an ssl client*/
        static SSL_METHOD ssl_method = {SSL_IS_CLIENT,
                                        2,
                                        SSL_MINOR_VERSION_0};
        ctx = (SSL_CTX*)calloc(1, sizeof(*ctx));
        ctx->ssl_method = &ssl_method;
        int ret = 10;
        int temp = 0;
        unsigned char cacert_buf[6000];
        memset(cacert_buf, 0, 6000);
        unsigned char swcert_buf[6000];
        memset(swcert_buf, 0, 6000);
        ocall_print_string(&temp, "###Enclave:ecall_ssl_ctx_new:loading certificates and key into context\n");

        ctx->CA_cert=*cacert;
        ctx->cert=*cert;
        ctx->pk=key;
        ocall_print_string(&temp, "###Enclave:ecall_ssl_ctx_new:Exiting Enclave\n");
        //check if keys and cert are there, probably a flag is needed, if so load them using mbedtls_***parse above
}

void
ecall_ssl_get_peer_certificate(mbedtls_x509_crt *cert) {
        int ret = 0;
        ocall_print_string(&ret, "###Enclave:ecall_ssl_get_peer_certificate:Inside enclave get peer\n");
        mbedtls_x509_crt *temp_cert= malloc(1024);
        mbedtls_x509_crt_init(temp_cert);
        char buf[1024];
        temp_cert = (mbedtls_x509_crt*)mbedtls_ssl_get_peer_cert(&ssl->cntx);
        memcpy(cert, temp_cert, 1024);
        mbedtls_x509_crt_info( (char *) buf, sizeof( buf ) - 1, "|-", cert );
        //ocall_print_string(&ret, "###Enclave:ecall_ssl_get_peer_certificate:Peer Certificate is\n");
        //ocall_print_string(&ret, buf);
        //ocall_print_string(&ret, "###Enclave:ecall_ssl_get_peer_certificate:Leaving Enclave \n");
}

int
ecall_ssl_shutdown() {
        int ret = 0;
        ret = mbedtls_ssl_close_notify(&ssl->cntx);
        return ret;
}

int
ecall_ssl_get_state() {
        int ret =0;
        if (ssl != NULL) {
                ret = ssl->cntx.state;
        } else {
                return 100;
        }

        if(ret == 16) {
                ret = 3;
        }
        return ret;
}


#ifdef __cplusplus
} // extern "C"
#endif
