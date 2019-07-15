#ifndef _H_HOTCALL_CONFIG_
#define _H_HOTCALL_CONFIG_

#include <stdint.h>

#define MAX_SPINLOCK_JOBS 5

struct hotcall_config {
    void (*execute_function_legacy)(uint8_t function_id, void **args, void *return_value);
    void (*execute_function)(uint8_t function_id, unsigned int n_iters, unsigned int n_params, void *args[n_iters][n_params]);
    void (*spin_lock_tasks[MAX_SPINLOCK_JOBS])();
    unsigned int spin_lock_task_count[MAX_SPINLOCK_JOBS];
    unsigned int spin_lock_task_timeouts[MAX_SPINLOCK_JOBS];
    unsigned int n_spinlock_jobs;
};

#endif
