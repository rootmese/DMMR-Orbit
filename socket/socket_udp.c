#include <socket_udp.h>

struct udp_receiver_data {
    struct sockaddr_in client_addr;
    socklen_t addr_len;
};

static struct udp_node *udp_pool = 0;
static unsigned udp_pool_size = 0;
static unsigned udp_pool_count = 0;

static void(*dispatcher_udp_cb)(protocol_base_cb*) = 0;

struct udp_node *get_udp_node(void) {
    register struct udp_node *p = udp_pool;
    register struct udp_node *p1 = udp_pool + udp_pool_size;
    for (; p < p1; ++p)
        if (!(p->run))
            return p;
    if (udp_pool_count >= udp_pool_size) {
        udp_pool_size = udp_pool_size ? udp_pool_size * 2 : 0x400;
        udp_pool = (struct udp_node*)realloc(udp_pool, udp_pool_size * sizeof(struct udp_node));
        if (!udp_pool)
            return 0;
    }
    return udp_pool + udp_pool_count++;
}

static void* receiver_thread(void* arg) {
    struct udp_node *node = (struct udp_node*)arg;
    struct timespec ts_monotonic, ts_realtime;

    do {
        struct node *n = get_node();
        struct iovec iov[1] = {
            { .iov_base = n->value, .iov_len = sizeof(n->value) }
        };

        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);

        struct msghdr msg = {
            .msg_name = &client_addr,
            .msg_namelen = addr_len,
            .msg_iov = iov,
            .msg_iovlen = 1,
            .msg_control = 0,
            .msg_controllen = 0,
            .msg_flags = 0
        };

        n->fd = node->node->fd;
        n->value_size = recvmsg(node->node->fd, &msg, 0);

        if (n->value_size <= 0) {
            if (node->run && n->value_size < 0)
                continue;
            else
                break;
        }

        n->proto = proto_t.proto_udp_t;
        n->port = ntohs(client_addr.sin_port);
        n->sin_addr = client_addr.sin_addr.s_addr;

        clock_gettime(CLOCK_MONOTONIC, &ts_monotonic);
        n->arrival = (uint64_t)ts_monotonic.tv_sec * 1000000000ULL + ts_monotonic.tv_nsec;

        clock_gettime(CLOCK_REALTIME, &ts_realtime);
        n->deadline = (uint64_t)ts_realtime.tv_sec * 1000000000ULL + ts_realtime.tv_nsec;

        if (node->on_receive_cb)
            node->on_receive_cb(n);

    } while (node->run);
    return 0;
}


ssize_t udp_send_to_client(struct node *n, size_t n_len) {
    if (!n || n_len == 0)
        return EOF;

    struct sockaddr_in dest_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(n->port),
        .sin_addr = { .s_addr = n->sin_addr }
    };

    struct node *n0 = n, *n1 = n0 + n_len;
    do {
        struct iovec iov[1] = {
            { .iov_base = n0->value, .iov_len = n0->value_size }
        };

        struct msghdr msg = {
            .msg_name = &dest_addr,
            .msg_namelen = sizeof(dest_addr),
            .msg_iov = iov,
            .msg_iovlen = 1,
            .msg_control = 0,
            .msg_controllen = 0,
            .msg_flags = 0
        };

        if (sendmsg(n0->fd, &msg, 0) < 0)
            return EOF;

    } while (++n0 < n1);

    return 0;
}

int udp_server_is_active(void){
    return (udp_pool) ? (!0) : (0);
}

int start_udp_service(struct udp_node *udp_node) 
{
    struct udp_node *node = udp_node;
    if (!node)
        return EOF;
    node->on_receive_cb = on_receive_cb;
    node->run = !0;
    if((node->node->fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        return EOF;
    }

    if(bind(node->node->fd, (struct sockaddr*)&server_addr, sizeof(server_addr))) {
        perror("bind failed");
        close(node->node->fd);
        return EOF;
    }

    if(pthread_create(&node->receiver_thread, 0, receiver_thread, node)) {
        perror("receiver thread creation failed");
        close(node->node->fd);
        return EOF;
    }
    return 0;
}

void disconnect_udp_service(struct udp_node *node){
    if(node){
        node->run = 0;
        sleep(1);
        memset(node, 0, sizeof(struct udp_node));
    }
}

int connect_udp_server(void) {
    int ret = udp_server_is_active();
    if(ret)
        return EOF;
    else{
        udp_pool = (struct udp_node*)calloc(0x400, sizeof(struct udp_node));
        if (!udp_pool)
            return EOF;
        udp_pool_size = 0x400;
        return 0;
    }
}

void disconnect_udp_server(void) {
    struct udp_node *p = udp_pool, *p1 = p + udp_pool_size;
    do {
        if (p) p->run = 0;
    } while (++p < p1);
    sleep(1);
    if (udp_pool)
        free(udp_pool);
}
