#ifndef __CALC_CIRCLE_BUFFER_H__
#define __CALC_CIRCLE_BUFFER_H__

static inline void calc_buffer(
    unsigned int interval_ms,
    unsigned int sleep_us,
    unsigned int conns,
    unsigned int bytes_per_ms,
    unsigned int *slots_per_conn,
    unsigned int *total_slots,
    unsigned int *total_bytes,
    unsigned int *total_bits
) {
    *slots_per_conn = (interval_ms * 1000) / sleep_us;
    *total_slots = (*slots_per_conn) * conns;
    *total_bytes = bytes_per_ms * interval_ms * conns;
    *total_bits = (*total_bytes) * 8;
}

#endif