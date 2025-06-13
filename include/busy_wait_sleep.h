#ifndef __BUSY_WAIT_SLEEP_H__
#define __BUSY_WAIT_SLEEP_H__

#include <time.h>
#include <stdint.h>

static inline void busy_wait_1_8us(void) {
    struct timespec start, now;
    clock_gettime(CLOCK_MONOTONIC, &start);
    do {
        clock_gettime(CLOCK_MONOTONIC, &now);
        uint64_t diff = (now.tv_sec - start.tv_sec)*1000000000ULL + (now.tv_nsec - start.tv_nsec);
        if (diff >= 1800) // 1800 ns = 1.8 µs
            break;
    } while(0);
}
#endif
