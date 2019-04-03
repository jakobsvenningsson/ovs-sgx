#include "rand.h"

uint32_t random_uint32() {
  uint32_t r;
  sgx_read_rand((unsigned char *) &r, 4);
  return r;
}

uint16_t random_uint16() {
  uint16_t r;
  sgx_read_rand((unsigned char *) &r, 2);
  return r;
}
