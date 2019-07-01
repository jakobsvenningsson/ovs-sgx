#ifndef _H_HOTCALL_BENCHMARK_
#define _H_HOTCALL_BENCHMARK_

#include <stdlib.h>

#define L1_CACHE_SIZE (1024 * 1024)

typedef unsigned long long ticks;

static inline ticks start (void) {
  unsigned cycles_low, cycles_high;
  asm volatile ("CPUID\n\t"
		"RDTSC\n\t"
		"mov %%edx, %0\n\t"
		"mov %%eax, %1\n\t": "=r" (cycles_high), "=r" (cycles_low)::
		"%rax", "%rbx", "%rcx", "%rdx");
  return ((ticks)cycles_high << 32) | cycles_low;
}

static inline ticks stop (void) {
  unsigned cycles_low, cycles_high;
  asm volatile("RDTSCP\n\t"
	       "mov %%edx, %0\n\t"
	       "mov %%eax, %1\n\t"
	       "CPUID\n\t": "=r" (cycles_high), "=r" (cycles_low):: "%rax",
	       "%rbx", "%rcx", "%rdx");
  return ((ticks)cycles_high << 32) | cycles_low;
}


static ticks b_time;
static ticks e_time;

#define BEGIN \
    b_time = start();

#define CLOSE \
    e_time = stop();

#define SHOWTIME printf("%lu\n", e_time - b_time);

#define SHOWTIME5 \
    { \
        char buf[32]; \
        int n = sprintf(buf, "%lu\n", e_time - b_time); \
        n = write(5, buf, n); \
    }

#define GET_TIME e_time - b_time;

static inline void
clear_cache() {
    long unsigned dummy[L1_CACHE_SIZE];
    for(int j = 0; j < L1_CACHE_SIZE; ++j) {
        dummy[j] = rand();
    }
}

int
cmpfunc (const void * a, const void * b);
void
benchmark(struct shared_memory_ctx *sm_ctx, unsigned int (*f_benchmark)(struct shared_memory_ctx *, unsigned int), unsigned int n_rounds, unsigned int n_iters);


unsigned int
benchmark_hotcall(struct shared_memory_ctx *sm_ctx, unsigned int n_rounds);
unsigned int
benchmark_variadic_function(struct shared_memory_ctx *sm_ctx, unsigned int n_rounds);

unsigned int
benchmark_if_naive(struct shared_memory_ctx *sm_ctx, unsigned int n_rounds);
unsigned int
benchmark_if_optimized(struct shared_memory_ctx *sm_ctx, unsigned int n_rounds);


unsigned int
benchmark_filter_naive(struct shared_memory_ctx *sm_ctx, unsigned int n_rounds);
unsigned int
benchmark_filter_optimized(struct shared_memory_ctx *sm_ctx, unsigned int n_rounds);


unsigned int
benchmark_map(struct shared_memory_ctx *sm_ctx, unsigned int n_rounds);
unsigned int
benchmark_filter(struct shared_memory_ctx *sm_ctx, unsigned int n_rounds);
unsigned int
benchmark_for_each(struct shared_memory_ctx *sm_ctx, unsigned int n_rounds);
unsigned int
benchmark_for(struct shared_memory_ctx *sm_ctx, unsigned int n_rounds);
#endif
