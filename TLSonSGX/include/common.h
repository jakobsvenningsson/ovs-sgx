#ifndef _SGX_COMMON_H
#define _SGX_COMMON_H

#include <sgx_spinlock.h>
#include <stdbool.h>
#include <stdarg.h>
#include <sgx_thread.h>

#define hotcall_ecall_ssl_write 0
#define hotcall_ecall_ssl_read 1
#define hotcall_ecall_ssl_library_init 2
#define hotcall_ecall_ssl_ctx_new 3
#define hotcall_ecall_ssl_new 4
#define hotcall_ecall_ssl_ctx_free 5
#define hotcall_ecall_ssl_ctx_set_verify 6
#define hotcall_ecall_ssl_get_peer_certificate 7
#define hotcall_ecall_ssl_load_error_strings 8
#define hotcall_ecall_ssl_free 9
#define hotcall_ecall_ssl_get_error 10
#define hotcall_ecall_ssl_set_fd 11
#define hotcall_ecall_ssl_connect 12
#define hotcall_ecall_ssl_accept 13
#define hotcall_ecall_ssl_get_state 14
#define hotcall_ecall_ssl_shutdown 15

typedef struct {
    int n_args;
    void *args[10];
} argument_list;


typedef struct {
  size_t allocated_size;
  void *val;
} return_value;

typedef struct {
  sgx_thread_mutex_t mutex;
  sgx_spinlock_t spinlock;
  sgx_thread_cond_t cond;
  bool run;
  bool running_function;
  bool is_done;
  bool sleeping;
  int timeout_counter;
  int function;
  argument_list *args;
  void *ret;
} async_ecall;

#endif
