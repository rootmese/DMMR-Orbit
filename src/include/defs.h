#ifndef __DEFS_H__
#define __DEFS_H__

#include <stdint.h> 
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <arpa/inet.h>
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

#include "mtu_config.h"

#define LOG() printf("[%s:%d]\n", __FUNCTION__, __LINE__)

#define LOG_ERRNO(msg) \
    fprintf(stderr, "[%s:%d] %s: (%d) %s\n", __FUNCTION__, __LINE__, msg, errno, strerror(errno))

#ifndef __SPAW_DETACHED_THREADS_H__
#define __SPAW_DETACHED_THREADS_H__

union protocol_base_cb;


static inline void spawn_detached_thread(pthread_t *t, void *(*fn)(void *), void *arg, int *ret)
{
    if (!(*ret = pthread_create(t, 0, fn, arg)))
        pthread_detach(*t);
    // TODO - colocar logs baseado em errno e strerror
}

#endif

#ifndef __SPAW_DETACHED_WITH_ATTRS_THREADS_H__
#define __SPAW_DETACHED_WITH_ATTRS_THREADS_H__

static inline void spawn_detached_thread_with_attr(pthread_t *t, pthread_attr_t *attr, void *(*fn)(void *), void *arg, int *ret)
{
    *ret = 0;
    if (!(*ret = pthread_create(t, attr, fn, arg)))
        pthread_detach(*t);
    // TODO - colocar logs baseado em errno e strerror
}

#endif


typedef enum{
    proto_none_t   = 0x00,
    proto_udp_t    = 0x01,
    proto_tcp_t    = 0x02
}proto_t;

typedef enum{
    proto_arr_udp_t    = 0x00,
    proto_arr_tcp_t    = 0x01
}proto_arr_t;

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

typedef enum {
    rb_log_level_fatal = 0x01,
    rb_log_level_error = 0x02,
    rb_log_level_warn  = 0x04,
    rb_log_level_info  = 0x08,
    rb_log_level_debug = 0x10
} rb_log_level;

struct node {
    uint64_t arrival;
    uint64_t deadline;
    uint32_t value_size;
    ezp_addr_type family;
    int fd;
    uint16_t port;
    uint8_t flags;
    uint8_t __filler0[1];
    struct sockaddr_in6 ipv6;
    struct sockaddr ipv4;
    uint8_t value[VALUE_BUFFER_SIZE];
};

struct node_buffer {
    uint8_t node_buffer_status;
    uint8_t __padding[7];
    struct node node;
};

struct node_recv_manager{
    uint8_t next_free_index;
    uint8_t next_busy_index;
	struct node_buffer buffer[0x100];
	pthread_mutex_t mutex;
};


struct none_node{
    proto_t proto;
};

struct tcp_node {
    proto_t proto;
    uint8_t run;
    uint8_t origin;
    uint8_t __filler0[2];
    uint32_t node_count;
    uint32_t __filler1;
    struct node *node;
    struct node node_cfg;
    pthread_t accept_thread;
    struct tcp_node *parent;
    struct node_recv_manager recv_manager;
    union protocol_base_cb *linked;
    struct iovec iovs[VALUE_OUTPUT_SIZE]; // TODO seis mede a relação MTU ethernet/Jumbo Frame
    void (*on_accept_cb)(struct tcp_node*);
    void (*on_connect_cb)(struct tcp_node*);
    void (*on_receive_cb)(struct node_recv_manager*);
    void (*on_close_cb)(struct tcp_node*); // TODO
};


struct udp_node {
    proto_t proto;
    uint8_t run;
    uint8_t origin;
    uint8_t __filler0[2];
    uint32_t node_count;
    uint32_t __filler1;
    struct node *node;
    struct node node_cfg;
    pthread_t accept_thread;
    struct node_recv_manager recv_manager;
    struct iovec iovs[VALUE_OUTPUT_SIZE]; // TODO seis mede a relação MTU ethernet/Jumbo Frame
    union protocol_base_cb *linked;
    void (*on_accept_cb)(struct udp_node*);
    void (*on_connect_cb)(struct udp_node*);
    void (*on_receive_cb)(struct node_recv_manager*);
    void (*on_close_cb)(struct udp_node*); // TODO
};


union protocol_base_cb{
    struct none_node none;
    struct tcp_node tcp;
    struct udp_node udp;
};

struct cfg_daemon_server {
    uint8_t  cfg_file[0x400];
    uint8_t daemonize;
};

struct cfg_server_server{
    uint64_t scheduler_preemptive_deadline;
    uint16_t sleep_time;
    uint16_t session_size;
    uint32_t circle_buffer_size;
    uint16_t max_ports;
    uint8_t log_level;
    uint8_t __filler1;
    uint64_t real_time_dead_line;
    uint64_t real_time_user_defined;
    uint8_t trunk_accept_uri[0x40];
    uint8_t trunk_dispatch_uri[0x40];
};

#ifndef __RB_LOG_ENABLE__
#define __RB_LOG_ENABLE__
static inline int rb_log_enabled(uint8_t __m, rb_log_level __l) {
    return (__m & __l);
}
#endif

#ifndef __RB_LOG_LEVEL_NAME__
#define __RB_LOG_LEVEL_NAME__
static inline const char* rb_log_level_name(rb_log_level __l) {
    switch(__l) {
        case rb_log_level_fatal:
            return "FATAL";
        case rb_log_level_error:
            return "ERROR";
        case rb_log_level_warn:
            return "WARN";
        case rb_log_level_info:
            return "INFO";
        case rb_log_level_debug:
            return "DEBUG";
        default:
            return "???";
    }
}
#endif

struct session_connection_pool {
    uint8_t run;
    uint16_t port;
    uint16_t linked_port;
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
    pthread_mutex_t mutex;
    struct session_connection_pool *session_ptr; // <-- ponteiro direto para a session_connection real
};

#ifndef __LINKED_SESSION_PORT_H__
#define __LINKED_SESSION_PORT_H__

static inline void link_session_ports(struct session_connection_pool *p, union protocol_base_cb *cb) {
    if (!p || !cb)
        return;

    p->port = 0;
    p->linked_port = 0;

    switch (cb->none.proto) {
        case proto_tcp_t:
            if (cb->tcp.node)
                p->port = cb->tcp.node->port;
            if (cb->tcp.linked && cb->tcp.linked->tcp.node)
                p->linked_port = cb->tcp.linked->tcp.node->port;
            break;

        case proto_udp_t:
            if (cb->udp.node)
                p->port = cb->udp.node->port;
            if (cb->udp.linked && cb->udp.linked->udp.node)
                p->linked_port = cb->udp.linked->udp.node->port;
            break;

        default:
            break;
    }
}

#endif

//

#ifndef __GET_SESSION_POINTER__
#define __GET_SESSION_POINTER__

static inline union protocol_base_cb *get_session_pointer(union protocol_base_cb *__u){
    if(__u){
        switch(__u->none.proto){
            case proto_none_t:
                return 0;
            case proto_tcp_t:
                return __u->tcp.linked;
                break; /* Stupid break */
            case proto_udp_t:
                return __u->udp.linked;
                break; /* Stupid break */
            default:
                return 0;
                break; /* Stupid break */
        }
    }
}

#endif

#ifndef __GET_SESSION_CFG__
#define __GET_SESSION_CFG__

static inline  void get_session_cfg(union protocol_base_cb *__u, uint16_t *__p0, proto_arr_t *__p1){
    if(__u){
        switch(__u->none.proto){
            case proto_none_t:
                break;
            case proto_tcp_t:
                *__p0 = __u->tcp.node->port;
                *__p1 = proto_arr_tcp_t;
                break;
            case proto_udp_t:
                *__p0 = __u->udp.node->port;
                *__p1 = proto_arr_udp_t;
                break;
            default:
                break; /* Stupid break */
        }
    }
}

#endif

#endif