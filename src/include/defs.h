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

#ifndef MTU_SIZE
#define MTU_SIZE 1500
#endif

#ifndef TOKEN_BUCKET_SIZE
#define TOKEN_BUCKET_SIZE 9000
#endif

#ifndef CIRCLE_BUFFER_SIZE
#define CIRCLE_BUFFER_SIZE 9000
#endif

#define LOG() printf("[%s:%d]\n", __FUNCTION__, __LINE__)

#define LOG_ERRNO(msg) \
    fprintf(stderr, "[%s:%d] %s: (%d) %s\n", __FUNCTION__, __LINE__, msg, errno, strerror(errno))

#ifndef __SPAW_DETACHED_THREADS_H__
#define __SPAW_DETACHED_THREADS_H__


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

struct node_buffer{
	uint8_t node_buffer_status;
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
    struct iovec iovs[0x06]; // TODO seis mede a relação MTU ethernet/Jumbo Frame
    void (*on_accept_cb)(struct tcp_node*);
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
    struct iovec iovs[0x06]; // TODO seis mede a relação MTU ethernet/Jumbo Frame
    void (*on_accept_cb)(struct udp_node*);
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

#endif