
#include <stdio.h>

#include <defs.h>
#include <__mset.h>
#include <__vcpy.h>
#include <node_recv_manager.h>

#include <socket_udp.h>

struct udp_receiver_data {
    struct sockaddr_in client_addr;
    socklen_t addr_len;
};

static struct udp_node *udp_pool = 0;
static unsigned udp_pool_size = 0;
static unsigned udp_pool_count = 0;

static void(*dispatcher_udp_cb)(union protocol_base_cb*) = 0;

static void* receiver_thread(void* arg) {
    struct udp_node *node = (struct udp_node*)arg;
    (void)init_node_recv_manager(&(node->recv_manager));
    struct timespec ts_monotonic, ts_realtime;
    union {
        struct sockaddr_in ipv4;
        struct sockaddr_in6 ipv6;
    } client_addr_storage;

    do {
        uint8_t pos;
        struct node *n =  get_free_node(&(node->recv_manager), &pos);
        struct iovec iov[1] = {
            { .iov_base = n->value, .iov_len = sizeof(n->value) }
        };
        socklen_t addr_len = (node->node_cfg.family == AF_INET6)
                             ? sizeof(client_addr_storage.ipv6)
                             : sizeof(client_addr_storage.ipv4);

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
            if (node->run && n->value_size == 0)
                continue;
            else
                break;
        }

        n->fd = node->node_cfg.fd;

        if (node->node_cfg.family == AF_INET) {
            n->port = ntohs(client_addr_storage.ipv4.sin_port);
            __vcpy(&(n->ipv4), &(client_addr_storage.ipv4.sin_addr), sizeof(struct in_addr));
        } else {
            n->port = ntohs(client_addr_storage.ipv6.sin6_port);
            __vcpy(&(n->ipv6), &(client_addr_storage.ipv6.sin6_addr), sizeof(struct in6_addr));
        }

        clock_gettime(CLOCK_MONOTONIC, &ts_monotonic);
        n->arrival = (uint64_t)ts_monotonic.tv_sec * 1000000000ULL + ts_monotonic.tv_nsec;
        clock_gettime(CLOCK_REALTIME, &ts_realtime);
        n->deadline = (uint64_t)ts_realtime.tv_sec * 1000000000ULL + ts_realtime.tv_nsec;

        if (node->on_receive_cb)
            node->on_receive_cb(&(node->recv_manager));

    } while (node->run);
    if(node->on_close_cb)
        node->on_close_cb(node);
    return 0;
}

int udp_send_to_client(struct udp_node *udp, struct node *n, size_t n_len) {
    if (!udp || !n || !n_len)
        return EOF;

    const int fd = n->fd;
    const sa_family_t family = n->family;

    union {
        struct sockaddr_in ipv4;
        struct sockaddr_in6 ipv6;
    } dest_addr;

    if (family == AF_INET) {
        dest_addr.ipv4.sin_family = AF_INET;
        dest_addr.ipv4.sin_port = htons(n[0].port);
        __vcpy(&dest_addr.ipv4.sin_addr, &n[0].ipv4, sizeof(struct in_addr));
    } else {
        dest_addr.ipv6.sin6_family = AF_INET6;
        dest_addr.ipv6.sin6_port = htons(n[0].port);
        __vcpy(&dest_addr.ipv6.sin6_addr, &n[0].ipv6, sizeof(struct in6_addr));
    }
    size_t siz = ((n_len > 6) ? (6) : (n_len));
    struct node *n0 = n, *n1 = n0 + siz;
    struct iovec *io0 = udp->iovs;
    do {
        io0->iov_base = n0->value;
        io0->iov_len = n0->value_size;
        ++io0;
    }while(++n0 < n1);

    struct msghdr msg = {0};
    msg.msg_name = &dest_addr;
    msg.msg_namelen = (family == AF_INET) ? sizeof(dest_addr.ipv4) : sizeof(dest_addr.ipv6);
    msg.msg_iov = udp->iovs;
    msg.msg_iovlen = siz;

    ssize_t sent = sendmsg(fd, &msg, 0);
    return (sent < 0 && errno != EAGAIN && errno != EINTR) ? EOF : 0;
}


int udp_server_is_active(void) {
    return (udp_pool) ? (!0) : (0);
}

int start_udp_service(struct udp_node *udp_node) {
    int ret;
    struct udp_node *node = udp_node;
    if (node) {
        int ret;
        int sock;

        node->run = !0;
        sock = node->node_cfg.fd = socket(
            (node->node_cfg.family == AF_INET) ? AF_INET : AF_INET6,
            SOCK_DGRAM,
            0
        );

        if (sock == EOF)
            goto return_error;

        switch (node->node_cfg.family) {
            case AF_INET:
                ret = bind(
                    node->node_cfg.fd,
                    (struct sockaddr *)&(node->node_cfg.ipv4),
                    sizeof(struct sockaddr_in)
                );
                if (ret)
                    goto return_socket_error;
                break;

            case AF_INET6:
                ret = bind(
                    node->node_cfg.fd,
                    (struct sockaddr *)&(node->node_cfg.ipv6),
                    sizeof(struct sockaddr_in6)
                );
                if (ret)
                    goto return_socket_error;
                break;
        }

        spawn_detached_thread(&node->accept_thread, receiver_thread, node, &ret);
        return ret;

    return_socket_error:
        close(node->node_cfg.fd);
    return_error:
        return EOF;
    }

    return EOF;
}

void disconnect_udp_service(struct udp_node *node) {
    if (node) {
        node->run = 0;
        sleep(1);
        __mset(node, 0, sizeof(struct udp_node));
    }
}

int connect_udp_server(void) {
    int ret = udp_server_is_active();
    if (ret)
        return EOF;

    udp_pool = (struct udp_node*)calloc(0x400, sizeof(struct udp_node));
    if (!udp_pool)
        return EOF;

    udp_pool_size = 0x400;
    return 0;
}

void disconnect_udp_server(struct udp_node *node) {
    struct udp_node *p = udp_pool, *p1 = p + udp_pool_size;
    do {
        if (p)
            p->run = 0;
    } while (++p < p1);

    sleep(1);

    if (udp_pool)
        free(udp_pool);
}

int start_udp_socket(void){
    return 0;
}
