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

typedef enum{
    proto_udp_t = 0x00;
    proto_tcp_t = 0x01;
}proto_t;

struct node {
    proto_t proto;
    uint16_t port;
    int fd;
	uint32_t sin_addr;
    unsigned value_size;
    unsigned char value[1500];
    uint64_t arrival;
    uint64_t deadline;
    uint8_t flags;
};


#endif