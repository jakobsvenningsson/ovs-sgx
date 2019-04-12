#ifndef _H_OVS_BENCHMARK
#define _H_OVS_BENCHMARK

#define BENCHMARK(FUNC, RETURN_VAR, ARGS...) \
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
    } while(0)

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(__cplusplus)
}
#endif

#endif
