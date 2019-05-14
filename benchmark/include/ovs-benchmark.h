#ifndef _H_OVS_BENCHMARK
#define _H_OVS_BENCHMARK


typedef unsigned long long ticks;

static __inline__ ticks start (void) {
  unsigned cycles_low, cycles_high;
  asm volatile ("CPUID\n\t"
		"RDTSC\n\t"
		"mov %%edx, %0\n\t"
		"mov %%eax, %1\n\t": "=r" (cycles_high), "=r" (cycles_low)::
		"%rax", "%rbx", "%rcx", "%rdx");
  return ((ticks)cycles_high << 32) | cycles_low;
}

static __inline__ ticks stop (void) {
  unsigned cycles_low, cycles_high;
  asm volatile("RDTSCP\n\t"
	       "mov %%edx, %0\n\t"
	       "mov %%eax, %1\n\t"
	       "CPUID\n\t": "=r" (cycles_high), "=r" (cycles_low):: "%rax",
	       "%rbx", "%rcx", "%rdx");
  return ((ticks)cycles_high << 32) | cycles_low;
}


/*#define BENCHMARK(FUNC, RETURN_VAR, ARGS...) \
    do { \
        struct timespec et; \
        struct timespec st; \
        clock_gettime(CLOCK_REALTIME, &st); \
        RETURN_VAR = FUNC(ARGS); \
        clock_gettime(CLOCK_REALTIME, &et); \
        { \
            char buf[32]; \
            int n = sprintf(buf, "%lu\n", et.tv_nsec - st.tv_nsec); \
            write(5, buf, n); \
        } \
    } while(0)*/



#define BENCHMARK(FUNC, RETURN_VAR, ARGS...) \
    do { \
        ticks et; \
        ticks st; \
        st = start(); \
        RETURN_VAR = FUNC(ARGS); \
        et = stop(); \
        { \
            char buf[32]; \
            int n = sprintf(buf, "%llu\n", et - st); \
            write(5, buf, n); \
        } \
    } while(0)


#define BENCHMARK_NO_RETURN(FUNC, ARGS...) \
    do { \
        struct timespec et; \
        struct timespec st; \
        clock_gettime(CLOCK_REALTIME, &st); \
        FUNC(ARGS); \
        clock_gettime(CLOCK_REALTIME, &et); \
        { \
            char buf[32]; \
            int n = sprintf(buf, "%lu\n", et.tv_nsec - st.tv_nsec); \
            write(5, buf, n); \
        } \
    } while(0)


/*
struct timespec b_time;
struct timespec e_time;
#define BEGIN \
    clock_gettime(CLOCK_REALTIME, &b_time);

#define CLOSE \
    clock_gettime(CLOCK_REALTIME, &e_time);

#define SHOWTIME printf("%lu\n", e_time.tv_nsec - b_time.tv_nsec);

#define SHOWTIME5 \
    { \
        char buf[32]; \
        int n = sprintf(buf, "%lu\n", e_time.tv_nsec - b_time.tv_nsec); \
        n = write(5, buf, n); \
    }
*/




ticks b_time;
ticks e_time;
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


#if defined(__cplusplus)
extern "C" {
#endif

#if defined(__cplusplus)
}
#endif

#endif
