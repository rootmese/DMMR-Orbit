#include <stdio.h>

#include <__mset.h>
#include <__mset.h>
#include <__vcpy.h>

#include <socket_tcp.h>

static struct tcp_node *tcp_pool = 0;
static unsigned tcp_pool_size = 0;
static unsigned tcp_pool_count = 0;

static struct node *node_pool = 0;
static unsigned node_pool_size = 0;
static unsigned node_pool_count = 0;

//TODO Faltam mutex no código
static struct node *get_node(void){
    register struct node *p = node_pool;
    register struct node *p1 = node_pool + node_pool_size;
    for (; p < p1; ++p)
        if (!(p->fd))
            return p;
    if(node_pool_count >= node_pool_size) {
        node_pool_size *= 2;
        node_pool = (struct node*)realloc(node_pool, node_pool_size * sizeof(struct node));
        if(!node_pool)
            return 0;
    }
    return node_pool + node_pool_count++;
}

int tcp_send_to_client(struct node *n,  size_t n_len) {
    if (!n || n_len == 0)
        return EOF;
    struct iovec iov[1];
    struct node *n0 = n, *n1 = n0 + n_len;
    do {
        iov[0].iov_base = n0->value;
        iov[0].iov_len = n0->value_size;
        ssize_t sent = writev(n0->fd, iov, 1);
        if (sent < 0) {
            switch(errno) {
                case EAGAIN:
                case EINTR:
                    break;
                default:
                    return EOF;
            }
        }
    } while(++n0 < n1);
    return 0;
}

static void* tcp_receiver_thread(void *arg) {
    struct tcp_node *tn = (struct tcp_node *)arg;
    struct timespec ts_monotonic, ts_realtime;
    struct iovec iov[1];
    if(!tn)
        goto done;
    do {
            struct node *n = tn->node;
            iov[0].iov_base = n->value;
            iov[0].iov_len = sizeof(n->value);
            n->value_size = readv(n->fd, iov, 1);
            if (n->value_size <= 0) {
                if (n->value_size < 0) {
                    switch (errno) {
                        case EAGAIN:
                        case EINTR:
                            continue; // Erro temporário, continua
                        case EPIPE:
                        case ECONNRESET:
                        case EBADF:
                        case ENOTCONN:
                        default:
                            goto done; // Erros críticos, encerra
                    }
                }
                else {
                    goto done;
                }
            }
            clock_gettime(CLOCK_MONOTONIC, &ts_monotonic);
            clock_gettime(CLOCK_REALTIME, &ts_realtime);
            n->arrival = (uint64_t)ts_monotonic.tv_sec * 1000000000ULL + ts_monotonic.tv_nsec;
            n->deadline = (uint64_t)ts_realtime.tv_sec * 1000000000ULL + ts_realtime.tv_nsec;
            if(tn->on_receive_cb)
                tn->on_receive_cb(n);
    } while(tn->run);

done:
    return 0;
}


static void *accept_thread(void *arg) {
    struct tcp_node *node = (struct tcp_node*)arg;
    struct sockaddr_in client_addr;
    struct sockaddr_in6 client_addr_v6;
    socklen_t addr_len = sizeof(client_addr);
    socklen_t addr_len_v6 = sizeof(client_addr_v6);
    do {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(node->node->fd, &readfds);
        struct timeval timeout = {
            .tv_sec = 1,
            .tv_usec = 0
        };
        int ret = select(node->node_cfg.fd + 1, &readfds, 0, 0, &timeout);
        if(ret <= 0)
            continue;
        if(FD_ISSET(node->node_cfg.fd, &readfds)) {
            int client_fd = accept(
                node->node_cfg.fd,
                (struct sockaddr *)(
                    (node->node_cfg.family == AF_INET)
                        ? (void *)&client_addr
                        : (void *)&client_addr_v6
                ),
                (node->node_cfg.family == AF_INET)
                    ? &addr_len
                    : &addr_len_v6
            );
            if(client_fd < 0) {
                if(node->run)
                    continue;
                else
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
                struct tcp_node *tn = get_tcp_node();
                if(tn){
                    tn->parent = node;
                    tn->node = n;
                    tn->node_count++;
                    tn->run = !0;
                    tn->on_receive_cb = node->on_receive_cb;
                    (void)pthread_create(&tn->accept_thread, 0, tcp_receiver_thread, tn);
                    if(node->on_accept_cb)
                        node->on_accept_cb(tn);
                }
            }
        }
    } while(node->run);
    return 0;
}

int connect_tcp_server(struct tcp_node *node){
    int ret;
    int sock;
    struct sockaddr_in client_addr;
    struct sockaddr_in6 client_addr_v6;
    socklen_t addr_len = sizeof(client_addr);
    socklen_t addr_len_v6 = sizeof(client_addr_v6);

    if (node) {
        sock = node->node_cfg.fd = socket(((node->node_cfg.family == AF_INET) ? AF_INET : AF_INET6), SOCK_STREAM, 0);
        if (sock == EOF)
            goto return_error;
        ret = connect(sock, (struct sockaddr *)&node->node_cfg.ipv4, sizeof(node->node_cfg.ipv4));
        if (ret)
            goto return_close_socket_error;

        struct node *n = get_node();
        if (n) {
            n->fd = sock;
            if (node->node_cfg.family == AF_INET) {
                n->port = ntohs(client_addr.sin_port);
                __vcpy(&(n->ipv4), &(client_addr.sin_addr), sizeof(struct in_addr));
            } else {
                struct sockaddr_in6 *a = &client_addr_v6;
                n->port = ntohs(a->sin6_port);
                __vcpy(&(n->ipv6), &(a->sin6_addr), sizeof(struct in6_addr));
            }
            struct tcp_node *tn = get_tcp_node();
            if (tn) {
                tn->node = n;
                tn->node_count++;
                tn->run = !0;
                tn->on_receive_cb = node->on_receive_cb;
                (void)pthread_create(&tn->accept_thread, 0, tcp_receiver_thread, tn);
                if (node->on_accept_cb)
                    node->on_accept_cb(tn);
            }
        }
    return 0;
    }

return_close_socket_error:
    close(sock);
return_error:
    return EOF;
}


struct tcp_node *get_tcp_node(void) {
    register struct tcp_node *p = tcp_pool;
    register struct tcp_node *p1 = tcp_pool + tcp_pool_size;
    for (; p < p1; ++p)
        if (!(p->run))
            return p;
    if(tcp_pool_count >= tcp_pool_size) {
        tcp_pool_size *= 2;
        tcp_pool = (struct tcp_node*)realloc(tcp_pool, tcp_pool_size * sizeof(struct tcp_node));
        if(!tcp_pool)
            return 0;
    }
    return tcp_pool + tcp_pool_count++;
}

void delete_tcp_node(struct tcp_node *node) {
    if (!node)
        return;
    for (struct tcp_node *p = tcp_pool, *end = tcp_pool + tcp_pool_size; p < end; ++p)
        if (p->run && p->parent == node)
            delete_tcp_node(p);
    node->parent = 0;
    sleep(1);
    __mset(node, 0, sizeof(struct tcp_node));
}

int start_tcp_service(struct tcp_node *node) {
    int sock;
    int ret;

    if (node) {
        sock = node->node_cfg.fd = socket(
            (node->node_cfg.family == AF_INET) ? AF_INET : AF_INET6,
            SOCK_STREAM,
            0
        );
        if (sock == EOF)
            goto return_error;

        int flags = fcntl(node->node_cfg.fd, F_GETFL, 0);
        ret = fcntl(node->node_cfg.fd, F_SETFL, flags | O_NONBLOCK);
        if (ret)
            goto return_socket_error;

        int opt = 1;
        ret = setsockopt(node->node_cfg.fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        if (ret)
            goto return_socket_error;

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

        ret = listen(node->node_cfg.fd, 64);
        if (ret)
            goto return_socket_error;

        node->run = !0;

        ret = pthread_create(&node->accept_thread, 0, accept_thread, node);
        if (ret) {
            close(node->node->fd);
            return EOF;
        }

        return 0;
    }

return_socket_error:
    close(sock);
return_error:
    return EOF;
}


void disconnect_tcp_server(struct tcp_node *node){
    if(node){
        node->run = 0;
        sleep(1);
        __mset(node, 0, sizeof(struct tcp_node));
    }
}

int start_tcp_socket(void) {
    tcp_pool = (struct tcp_node*)calloc(0x400, sizeof(struct tcp_node));
    if(!tcp_pool)
        return EOF;
    tcp_pool_size = 0x400;
    node_pool = (struct node*)calloc(0x400, sizeof(struct node));
    if(!node_pool){
        free(tcp_pool);
        return EOF;
    }
    node_pool_size = 0x400;
    return 0;
}

void stop_tcp_socket(void) {
    struct tcp_node *p = tcp_pool, *p1 = p + tcp_pool_size;
    do{
        if(p)
            p->run = 0;
    }while(++p < p1);
    sleep(1);
    if(tcp_pool)
        free(tcp_pool);
}

