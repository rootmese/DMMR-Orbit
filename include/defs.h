#ifndef __DEFS_H__
#define __DEFS_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/errno.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>

#include <dmmr_circle_buffer.h>
#include <dmmr_parser.h>
#include <dmmr_scheduler.h>
#include <dmmr_server.h>
#include <session_connection.h>
#include <dmmr_session_connection_manager.h>
#include "__vcpy.h"
#include "__bcpy.h"
#include "crc.h"
#include "update_session_counter.h"
#include "__node_cmp.h"
#include "__mset.h"

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
    proto_none_t   = 0x00,
    proto_udp_t    = 0x01,
    proto_tcp_t    = 0x02,
}proto_t;

typedef enum {
    EZP_DNS     = 0,
    EZP_IPV4    = AF_INET,
    EZP_IPV6    = AF_INET6,
    EZP_INVALID = 0xFFFF
} ezp_addr_type;

struct node {
    uint64_t arrival;
    uint64_t deadline;
    struct sockaddr_in6 ipv6;
    struct sockaddr ipv4;
    int fd;
    uint32_t __filler0;
    ezp_addr_type family;
    uint16_t port;
    uint32_t value_size;
    uint8_t flags;
    uint8_t __filler2[7];
    uint8_t value[1500];
};


struct none_node{
    proto_t proto;
};

struct tcp_node {
    proto_t proto;
    uint8_t run;
    uint8_t origin;            // 0x3 = tcp[0], 0x4 = tcp[1]
    uint8_t __filler0[2];      // alinhamento
    uint32_t node_count;
    uint32_t __filler1;
    struct node *node;
    struct node node_cfg;
    pthread_t accept_thread;
    struct tcp_node *parent;
    void (*on_accept_cb)(struct tcp_node*);
    void (*on_receive_cb)(struct node*);
};

struct udp_node {
    proto_t proto;
    uint8_t run;
    uint8_t origin;            // 0x1 = udp[0], 0x2 = udp[1]
    uint8_t __filler0[2];      // alinhamento
    uint32_t node_count;
    uint32_t __filler1;
    struct node *node;
    struct node node_cfg;
    pthread_t accept_thread;
    void (*on_accept_cb)(struct udp_node*);
    void (*on_receive_cb)(struct node*);
}; 

union protocol_base_cb{
    struct none_node none;
    struct tcp_node tcp;
    struct udp_node udp;
};


#endif