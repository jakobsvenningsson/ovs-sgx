#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

#include <sgx_spinlock.h>

uint32_t sgx_spin_unlock(sgx_spinlock_t *lock);
uint32_t sgx_spin_lock(sgx_spinlock_t *lock);

#endif
