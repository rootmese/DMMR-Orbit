#ifndef __CALC_SLOT_SLEEP_US_H__
#define __CALC_SLOT_SLEEP_US_H__


#include <stdint.h>
#include <stdio.h>
#include <math.h>

static inline useconds_t calculate_slot_sleep_us(unsigned circle_buffer_size, unsigned session_count, double realtime_target_us) {
    const double overhead_per_session_ns = 270.0;
    double total_overhead_us = (overhead_per_session_ns * session_count) / 1000.0;
    if (realtime_target_us <= total_overhead_us)
        return 0;
    double sleep_us = (realtime_target_us - total_overhead_us) / circle_buffer_size;
    return (useconds_t)round(sleep_us);
}

#endif
