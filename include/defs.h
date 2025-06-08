#ifndef __DEFS_H__
#define __DEFS_H__

#include <sys/types.h>

#ifndef MTU_SIZE
#define MTU_SIZE 1500
#endif

#ifndef TOKEN_BUCKET_SIZE
#define TOKEN_BUCKET_SIZE 9000
#endif

#ifndef CIRCLE_BUFFER_SIZE
#define CIRCLE_BUFFER_SIZE 9000
#endif

struct node {
    uint16_t port;
    int fd;
    unsigned value_size;
    unsigned char value[MTU_SIZE];

    uint64_t arrival;   // CLOCK_MONOTONIC timestamp (ns ou us)
    uint64_t deadline;  // Calculado pelo SessionConnection
    uint8_t  flags;     // Ex: PRIORITY, DROPPABLE, RETRANSMIT, etc.
};

#endif