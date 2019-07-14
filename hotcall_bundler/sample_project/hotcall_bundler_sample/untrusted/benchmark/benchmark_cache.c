#include "benchmark.h"
#include "hotcall-bundler-untrusted.h"
#include "functions.h"
#include "sample.h"
#include "hotcall-hash.h"
#include "hotcall.h"


unsigned int
benchmark_cache(struct shared_memory_ctx *sm_ctx, unsigned int n_rounds) {
    unsigned int warmup = n_rounds / 10;
    unsigned int rounds[n_rounds];
    for(int i = 0; i < (n_rounds + warmup); ++i) {
        clear_cache();
        int x = 0, y = 0, counter = 0, res;
        const uint32_t tmp[] = { x, y };

        BEGIN

        uint32_t hash = hcall_hash_words(tmp, 2, 0);
        HCALL(
            CONFIG(.function_id = hotcall_ecall_add_and_count, .has_return = true, .memoize = { .on = true, .return_type = 'd', .hash = hash }),
            VAR(x, 'd'), VAR(y, 'd'), PTR(&counter), VAR(res, 'd'));



        CLOSE

        if(i >= warmup) {
            rounds[i - warmup] = GET_TIME
        }

        unsigned int t = GET_TIME
        printf("T: %u\n", t);
    }
    qsort(rounds, n_rounds, sizeof(unsigned int), cmpfunc);
    return rounds[n_rounds / 2];
}
