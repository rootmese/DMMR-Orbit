#ifndef __DMMR_SLEEP_H__
#define __DMMR_SLEEP_H__

#define NSLEEP_US(us) do { \
    struct timespec ts; \
    clock_gettime(CLOCK_MONOTONIC, &ts); \
    ts.tv_nsec += (us) * 1000; \
    if (ts.tv_nsec >= 1000000000) { ts.tv_sec++; ts.tv_nsec -= 1000000000; } \
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, NULL); \
} while(0)

#endif