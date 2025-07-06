#ifndef __STRUCT_H__
#define __STRUCT_H__

#include <enums.h>

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

#endif
