#ifndef __DMMR_SLEEP_H__
#define __DMMR_SLEEP_H__

#include <time.h>

static inline void nsleep_us(unsigned long us) {
    struct timespec ts;
#ifdef __FreeBSD__
    clock_gettime(CLOCK_MONOTONIC_FAST, &ts);
#else
    clock_gettime(CLOCK_MONOTONIC, &ts);
#endif
    ts.tv_nsec += us * 1000;
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000;
    }
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, NULL);
}

#define NSLEEP_US(x) nsleep_us(x)

#endif
