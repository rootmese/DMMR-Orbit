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
    union {
        struct sockaddr_in ipv4;
        struct sockaddr_in6 ipv6;
    } client_addr_storage;
    do {
        struct node *n = node->node;
        struct iovec iov[1] = {
            { .iov_base = n->value, .iov_len = sizeof(n->value) }
        };
        socklen_t addr_len;
        if (node->node_cfg.family == AF_INET6) {
            addr_len = sizeof(client_addr_storage.ipv6);
        }
        else {
            addr_len = sizeof(client_addr_storage.ipv4);
        }
        struct msghdr msg = {
            .msg_name = &client_addr_storage,
            .msg_namelen = addr_len,
            .msg_iov = iov,
            .msg_iovlen = 1,
            .msg_control = 0,
            .msg_controllen = 0,
            .msg_flags = 0
        };
        n->value_size = recvmsg(node->node_cfg.fd, &msg, 0);
        if (n->value_size <= 0) {
            if (node->run && n->value_size < 0)
                continue;
            else
                break;
        }
        n->fd = node->node_cfg.fd;
        n->proto = proto_t.proto_udp_t;
        if (node->node_cfg.family == AF_INET) {
            n->port = ntohs(client_addr_storage.ipv4.sin_port);
            __vcpy(&(n->ipv4), &(client_addr_storage.ipv4.sin_addr), sizeof(struct in_addr));
        }
        else {
            n->port = ntohs(client_addr_storage.ipv6.sin6_port);
            __vcpy(&(n->ipv6), &(client_addr_storage.ipv6.sin6_addr), sizeof(struct in6_addr));
        }
        clock_gettime(CLOCK_MONOTONIC, &ts_monotonic);
        n->arrival = (uint64_t)ts_monotonic.tv_sec * 1000000000ULL + ts_monotonic.tv_nsec;
        clock_gettime(CLOCK_REALTIME, &ts_realtime);
        n->deadline = (uint64_t)ts_realtime.tv_sec * 1000000000ULL + ts_realtime.tv_nsec;
        if (node->on_receive_cb)
            node->on_receive_cb(n);
    } while (node->run);
    return 0;
}



int udp_send_to_client(struct node *n, size_t n_len) {
    if (!n || n_len == 0)
        return EOF;
    struct node *n0 = n, *n1 = n0 + n_len;
    do {
        struct iovec iov[1] = {
            { .iov_base = n0->value, .iov_len = n0->value_size }
        };
        union {
            struct sockaddr_in ipv4;
            struct sockaddr_in6 ipv6;
        } dest_addr_storage;
        struct msghdr msg;
        memset(&msg, 0, sizeof(msg));
        if (n0->family == AF_INET) {
            dest_addr_storage.ipv4.sin_family = AF_INET;
            dest_addr_storage.ipv4.sin_port = htons(n0->port);
            dest_addr_storage.ipv4.sin_addr = n0->ipv4;
            msg.msg_name = &dest_addr_storage.ipv4;
            msg.msg_namelen = sizeof(dest_addr_storage.ipv4);
        }
        else if (n0->family == AF_INET6) {
            dest_addr_storage.ipv6.sin6_family = AF_INET6;
            dest_addr_storage.ipv6.sin6_port = htons(n0->port);
            dest_addr_storage.ipv6.sin6_addr = n0->ipv6;
            msg.msg_name = &dest_addr_storage.ipv6;
            msg.msg_namelen = sizeof(dest_addr_storage.ipv6);
        }
        else
            return EOF;
        msg.msg_iov = iov;
        msg.msg_iovlen = 1;
        (void)sendmsg(n0->fd, &msg, 0);
    } while (++n0 < n1);

    return 0;
}


int udp_server_is_active(void){
    return (udp_pool) ? (!0) : (0);
}

int start_udp_service(struct udp_node *udp_node) 
{
    struct udp_node *node = udp_node;
    if (node){
        înt ret;
        node->on_receive_cb = on_receive_cb;
        node->run = !0;
        sock = node->node_cfg.fd = socket(AF_INET, SOCK_DGRAM, 0));
        if(sock == EOF)
            goto return_error;
        switch(node->node_cfg.family){
            case AF_INET:{
                ret = bind(node->node_cfg.fd, (struct sockaddr*)&(node->node_cfg.ipv4), sizeof(server_addr));
                if(ret)
                    goto return_socket_error;
            }
                break;
            case AF_INET6:{
                ret = bind(node->node_cfg.fd, (struct sockaddr_in6*)&(node->node_cfg.ipv6), sizeof(server_addr));
                if(ret)
                    goto return_socket_error;
            }
                break;
        }
        struct node *n = get_node();
        if(n){
         n->fd = client_fd;
        if (node->node_cfg.family == AF_INET) {
            n->port = ntohs(client_addr.sin_port);
            __vcpy(&(n->ipv4), &(client_addr.sin_addr), sizeof(struct in_addr));
        }
        else {
            struct sockaddr_in6 *a = &client_addr_v6;
            n->port = ntohs(a->sin6_port);
            __vcpy(&(n->ipv6), &(a->sin6_addr), sizeof(struct in6_addr));
        }
        n->run = !0;
        struct tcp_node *tn = get_tcp_node();
        if(tn){
            tn->node = n;
            tn->node_count++;
            tn->on_dispatch_cb = node->on_dispatch_cb;
            (void)pthread_create(&tn->thread, 0, tcp_receiver_thread, tn);
            if(node->on_accept_cb)
                node->on_accept_cb(tn);
        }
        return 0;
    }
    return_close_socket_error:
        close(sockfd);
    return_error:
        return EOF;
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
