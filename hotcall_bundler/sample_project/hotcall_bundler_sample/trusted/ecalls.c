#include "ecalls.h"
#include <string.h>
#include <stdint.h>

bool
ecall_always_true() {
  return true;
}

bool
ecall_always_false() {
  return false;
}

void
ecall_foo() {
}

void
ecall_bar() {}

void
ecall_plus_one(int *x) {
    ++*x;
}

int
ecall_plus_one_ret(int x) {
    return ++x;
}

bool
ecall_greater_than_two(int *x) {
    return *x > 2 ? true : false;
}


bool
ecall_greater_than_y(int *x, int y) {
    return *x > y ? true : false;
}

void
ecall_plus_plus(int *x, int *y) {
    *x = *x + 1;
    *y = *y + 2;
}

int
ecall_plus(int x, int y) {
    return x + y;
}

void
ecall_plus_y(int *x, int y) {
    *x = *x + y;
}

void
ecall_plus_y_v2(int x, int *y) {
    *y = x + *y;
}

bool
ecall_revert(bool x) {
    return x == true ? false : true;
}

void
ecall_zero(int *x) {
    *x = 0;
}

int buffer[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

int
ecall_count() {
    return sizeof(buffer) /sizeof(int);
}

int
ecall_read_buffer(int *out, int size) {
    unsigned buffer_size = ecall_count();
    int n = (buffer_size > size ? size : buffer_size);
    memcpy(out, buffer, n * sizeof(int));
    return n;
}

void
ecall_change_ptr_to_ptr(int **p2p, int *p) {
    *p2p = p;
}

void
ecall_offset_of(void **ptr, int offset) {
    *ptr = ((char *) *ptr) + offset;
}

void *
ecall_offset_of_ret(void *ptr, unsigned int offset) {
    return ((char *) ptr) + offset;
}

int
ecall_strlen(struct A *a) {
    return a->x;
}

void
ecall_for_each_10_test(uint8_t bridge_id, uint8_t table_id, int *buf, unsigned grp_prio, unsigned int user_prio) {
    if(bridge_id++!= 0) printf("error bridge\n");
    if(table_id++ != 1) printf("error table\n");
    if(grp_prio++ != 100) printf("error grp prio\n");
    if(user_prio != 9 && user_prio != 8 && user_prio != 7) printf("error user_prio\n");
    user_prio++;
    *buf = *buf + 1;
}

int
ecall_add_and_count(int x, int y, int *counter) {
    ++*counter;
    return x + y;
}

void *
ecall_get_addr(void *x) {
    return x;
}
