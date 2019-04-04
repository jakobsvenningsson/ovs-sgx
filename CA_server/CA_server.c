#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <stdlib.h>
#include <string>

using namespace std;

//char goodMeasurement[128]= "sha1:1de086c120ee73358c468f29545af52e4e676bec";
char goodMeasurement[256]= "sha256:TEST_MEASSUREMENT";

int create_socket(int port)
{
    int s;
    struct sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
	perror("Unable to create socket");
	exit(EXIT_FAILURE);
    }

    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
	perror("Unable to bind");
	exit(EXIT_FAILURE);
    }

    if (listen(s, 1) < 0) {
	perror("Unable to listen");
	exit(EXIT_FAILURE);
    }

    return s;
}

void init_openssl()
{
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

void cleanup_openssl()
{
    EVP_cleanup();
}

SSL_CTX *create_context()
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = SSLv23_server_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) {
	perror("Unable to create SSL context");
	ERR_print_errors_fp(stderr);
	exit(EXIT_FAILURE);
    }

    return ctx;
}

void configure_context(SSL_CTX *ctx)
{
    SSL_CTX_set_ecdh_auto(ctx, 1);

    /* Set the key and cert */
    if (SSL_CTX_use_certificate_file(ctx, "rootca-cert.pem", SSL_FILETYPE_PEM) < 0) {
        ERR_print_errors_fp(stderr);
	exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "rootca-key.pem", SSL_FILETYPE_PEM) < 0 ) {
        ERR_print_errors_fp(stderr);
	exit(EXIT_FAILURE);
    }
}



int main(int argc, char **argv)
{
    int sock;
    SSL_CTX *ctx;
    char ima_buf[1012];
    char req_buf[8196];

    init_openssl();
    ctx = create_context();
    configure_context(ctx);
    sock = create_socket(4433);

    /* Handle connections */
    while(1) {

        struct sockaddr_in addr;
        uint len = sizeof(addr);
        SSL *ssl;

        int client = accept(sock, (struct sockaddr*)&addr, &len);

        printf("New client connection.\n");

        if (client < 0) {
            perror("Unable to accept");
            exit(EXIT_FAILURE);
        }

        ssl = SSL_new(ctx);
        SSL_set_fd(ssl, client);

        if (SSL_accept(ssl) <= 0) {
            ERR_print_errors_fp(stderr);
            SSL_free(ssl);
            close(client);
            continue;
        }

        char verdict[4];

        // Read IMA measurement
        int ima_length = SSL_read(ssl, ima_buf, strlen(goodMeasurement) + 3);
        ima_buf[ima_length] = '\0';

        if (strncmp(ima_buf, "IMA", 3) != 0 || strncmp(ima_buf + 3, goodMeasurement, ima_length - 3) != 0) {
          printf("IMA is not valid!\n");
          strcpy(verdict, "NOK");
          SSL_write(ssl, verdict, 4);
          SSL_free(ssl);
          close(client);
          continue;
        }

        printf("IMA is valid!\n");
        strcpy(verdict, "MOK");
        SSL_write(ssl, verdict, 4);

        // Read SSL signing request into buffer
  			int req_length = 0;
			  while (!strcmp(req_buf, "")){
      	   req_length = SSL_read(ssl, req_buf, sizeof(req_buf));
           printf("Inside loop, reading %d bytes\n", req_length);
   	       req_buf[req_length] = 0;
        }

        printf("Read %d bytes into req_buf\n", req_length);

        // Write SSL request to file
    		char current_dir[1024];
    		getcwd(current_dir, sizeof(current_dir));

        // Construct sign request file string
        char sign_request_file_str[128];
        strcpy(sign_request_file_str, current_dir);
        strcat(sign_request_file_str, "/sc-req.pem");

        FILE *req_file;
    		req_file = fopen(sign_request_file_str, "wb");
    		fwrite(req_buf, sizeof(req_buf), 1, req_file);
    		fclose(req_file);

        // Sign request
    		system("openssl ca -passin pass:pw123  -batch -config openssl.cnf -out sc-cert.pem -infiles sc-req.pem");

        remove(sign_request_file_str);


        char cert_file_str[128];
        strcpy(cert_file_str, current_dir);
        strcat(cert_file_str, "/sc-cert.pem");

        FILE *cert_file;
    		cert_file = fopen(cert_file_str, "rb");

        // Get size of certificate
    		long cert_size;
    		fseek(cert_file, 0, SEEK_END);
    		cert_size = ftell(cert_file);
    		rewind(cert_file);

    		char *cert_buf = (char*) malloc(sizeof(char) * cert_size);
    		int n = fread(cert_buf, sizeof(char), cert_size, cert_file);
        cert_buf[n] = '\0';

    		printf("Generated Certificate to be sent is %s\n", cert_buf);
        printf("Size of Certificate is %d\n", n);

    		SSL_write(ssl, cert_buf, n);

    		free(cert_buf);
        fclose(cert_file);
        remove(cert_file_str);
        SSL_free(ssl);
        close(client);
    }

    close(sock);
    SSL_CTX_free(ctx);
    cleanup_openssl();
}
