#ifndef _H_HOTCALL_CONFIG_
#define _H_HOTCALL_CONFIG_

#include "hotcall_function.h"

#define MAX_SPINLOCK_JOBS 5

struct hotcall_config {
    void (*execute_function)(struct hotcall_function_ *);
    void (*spin_lock_tasks[MAX_SPINLOCK_JOBS])();
    unsigned int spin_lock_task_count[MAX_SPINLOCK_JOBS];
    unsigned int spin_lock_task_timeouts[MAX_SPINLOCK_JOBS];
    unsigned int n_spinlock_jobs;
};

#endif
