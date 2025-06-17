#ifndef __DEFS_H__
#define __DEFS_H__

#include <stdint.h> 
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/errno.h>
#include <sys/queue.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/wait.h>

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
    proto_tcp_t    = 0x02
}proto_t;

typedef enum {
    EZP_DNS     = 0,
    EZP_IPV4    = AF_INET,
    EZP_IPV6    = AF_INET6,
    EZP_INVALID = 0xFFFF
} ezp_addr_type;

typedef enum {
    TOKEN_UNKNOWN,
    TOKEN_KEYWORD,
    TOKEN_IDENTIFIER,
    TOKEN_IPADDR,
    TOKEN_EQUALS
} dmmt_token_type;

struct node {
    uint64_t arrival;
    uint64_t deadline;
    struct sockaddr_in6 ipv6;
    struct sockaddr    ipv4;
    int fd;
    uint32_t __filler0;
    ezp_addr_type family;
    uint16_t port;
    uint16_t __filler1;
    uint32_t value_size;
    uint8_t flags;
    uint8_t __filler2[3];
    uint8_t value[1420];
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


struct cfg_daemon_server {
    uint8_t  cfg_file[0x400];
};

struct cfg_server_server{
    uint64_t scheduler_preemptive_deadline;
    uint16_t sleep_time;
    uint16_t session_size;
    uint32_t circle_buffer_size;
    uint16_t max_ports;
    uint16_t __filler1;
    uint64_t real_time_dead_line;
    uint64_t real_time_user_defined;
    uint8_t trunk_accept_uri[0x40];
    uint8_t trunk_dispatch_uri[0x40];
};

struct session_connection_pool {
    uint8_t run;
    uint16_t port;
    uint32_t pool_size;
    uint32_t pool_count;
    struct node *pool;
    struct node_circle_buffer *cursor;
    union protocol_base_cb session;
    pthread_t thread;
    pthread_mutex_t mutex;
};

struct scheduler_connection {
    uint64_t last_active_time_us;
    uint64_t realtime_deadline_us;
    uint64_t deadline_us;
    struct session_connection_pool *session_ptr; // <-- ponteiro direto para a session_connection real
};

#endif