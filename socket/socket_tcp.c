#include <socket_tcp.h>

static struct tcp_node *tcp_pool = 0;
static unsigned tcp_pool_size = 0;
static unsigned tcp_pool_count = 0;

ssize_t tcp_send_to_client(struct node *n,  size_t n_len) {
    if (!n || n_len == 0)
        return EOF;
    struct iovec iov[1];
    struct node *n0 = n, *n1 = n0 + n_len;
    do {
        iov[0].iov_base = n0->value;
        iov[0].iov_len = n0->value_size;
        ssize_t sent = writev(n->fd, &iov, 1);
        if (sent < 0) {
            switch(errno) {
                case EAGAIN:
                case EINTR:
                    return 0; // Não foi enviado, mas não é erro crítico
                default:
                    return EOF;
            }
        }
    } while(++n0 < n1);
    return 0;
}


static void* tcp_receiver_thread(void *arg) {
    struct tcp_node *n = (struct tcp_node *)arg;
    struct timespec ts_monotonic, ts_realtime;
    struct iovec iov[1];
    if(!n)
        goto done;
    do {
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
            n->node.proto = proto_t.proto_tcp_t;
            n->node.port = htons(node->node.port);
            n->node.arrival = (uint64_t)ts_monotonic.tv_sec * 1000000000ULL + ts_monotonic.tv_nsec;
            n->node.deadline = (uint64_t)ts_realtime.tv_sec * 1000000000ULL + ts_realtime.tv_nsec;
            if(n->on_accept_cb)
                n->on_accept_cb(n);
    } while(n->run);

done:
    return 0;
}


static void *accept_thread(void *arg) {
    struct tcp_node *node = (struct tcp_node*)arg;
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    do {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(node->node.fd, &readfds);

        struct timeval timeout = {
            .tv_sec = 1,
            .tv_usec = 0
        };

        int ret = select(node->node.fd + 1, &readfds, 0, 0, &timeout);
        if(ret <= 0)
            continue;

        if(FD_ISSET(node->node.fd, &readfds)) {
            int client_fd = accept(node->node.fd, (struct sockaddr*)&client_addr, &addr_len);
            if(client_fd < 0) {
                if(node->run)
                    continue;
                else
                    break;
            }
            struct node *n = node->node;
            if(n){
                n->fd = client_fd;
                n->port = htons(client_addr.sin_port);
                __vcpy(&(n->sin_addr), &(client_addr.sin_addr.s_addr), sizeof(struct sockaddr));
                n->run = !0;
                pthread_create(&node->thread, 0, tcp_receiver_thread, node);
            }
            node->node.fd = client_fd;
            node->node.port = htons(client_addr.sin_port);
            node->node.sin_addr = client_addr.sin_addr.s_addr;
            if(node->on_dispatch_cb)
                node->on_dispatch_cb(&node->node);
        }
    } while(node->run);
    return 0;
}

// TODO Avoid memleak recicle closed connection
/*
    Something like this:
       struct tcp_node *p = tcp_pool; *p1 = p + tcp_pool_size;
       do{
            if(!p)
                return p;
       }while(++p < p1);
*/
struct tcp_node *get_tcp_node(void) {
    if(tcp_pool_count >= tcp_pool_size) {
        tcp_pool_size *= 2;
        tcp_pool = (struct tcp_node*)realloc(tcp_pool, tcp_pool_size * sizeof(struct tcp_node));
        if(!tcp_pool)
            return 0;
    }
    return tcp_pool + tcp_pool_count++;
}

int start_tcp_service(struct tcp_node *node) {
    int sock;
    int ret;
    if(node){
        sock = node->node.fd = socket(AF_INET, SOCK_STREAM, 0);
        if(sock == EOF)
            goto return_error;
        int flags = fcntl(node->node.fd, F_GETFL, 0);
        ret = fcntl(node->node.fd, F_SETFL, flags | O_NONBLOCK);
        if(ret)
            goto return_socket_error;
        int opt = 1;
        ret = setsockopt(node->node.fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        if(ret)
            goto return_socket_error;
        //TODO use generica server_addr
        struct sockaddr_in server_addr = {
            .sin_family = AF_INET,
            .sin_addr.s_addr = INADDR_ANY,
            .sin_port = htons(port)
        };
        ret = bind(node->node.fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
        if(ret)
            goto return_socket_error;
        ret = listen(node->node.fd, 64));
        if(ret)
            goto return_socket_error;
        node->run = !0;
        if(pthread_create(&node->accept_thread, 0, accept_thread, node)) {
            perror("accept thread creation failed");
            close(node->node.fd);
            return EOF;
        }
        return 0;
    }
    return_socket_error:
        close(sock);
    return_error:
        return EOF;
}

int connect_tcp_server(struct tcp_node *tcp_node, const char *ip){
    if(!(tcp_node))
        return EOF;
    struct node *n = &(tcp_node->node);
    if(n){
        int ret;
        int sockfd;
        ezp_addr_type n_ret;
        unsigned char ipaddr_buffer[sizeof(struct sockaddr_in6)];
        struct sockaddr_in server_addr;

        n_ret = ezp_addr_type dns2ipaddr(ip, ipaddr_buffer);
        switch(n_ret){
            case EZP_IPV4:{
                struct sockaddr_in *server_addr = (struct sockaddr_in*)ipaddr_buffer;
                sockfd = socket(AF_INET, SOCK_STREAM, 0);
                if(sockfd < 0) 
                    goto return_error;
                memset(server_addr, 0, sizeof(struct sockaddr_in));
                server_addr.sin_family = AF_INET;
                server_addr.sin_port = htons(tcp_node->node.port);
                ret = connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
                if(ret)
                    goto return_close_socket_error;
                else{
                    n->fd = sockfd;
                    n->port =  htons(tcp_node->node.port);
                    __vcpy(&(n->ipv4), &(server_addr->sin_addr), sizeof(struct in_addr_in));
                    n->run = !0;
                    pthread_create(&n->thread, 0, tcp_receiver_thread, tcp_node);
                }
            }
                break; /* Stupid break */
            case EZP_IPV6:{
                struct sockaddr_in6 *server_addr6 = (struct sockaddr_in6 *)ipaddr_buffer;
                sockfd = socket(AF_INET6, SOCK_STREAM, 0);
                if (sockfd < 0)
                    goto return_error;
                memset(server_addr6, 0, sizeof(struct sockaddr_in6));
                server_addr6->sin6_family = AF_INET6;
                server_addr6->sin6_port = htons(tcp_node->node.port);
                int ret = connect(sockfd, (struct sockaddr*)server_addr6, sizeof(struct sockaddr_in6));
                if (ret)
                    goto return_close_socket_error;
                else {
                    n->fd = sockfd;
                    n->port = htons(tcp_node->node.port);
                    __vcpy(&(n->ipv6), &(server_addr6->sin6_addr), sizeof(struct in6_addr));
                    n->run = !0;
                    pthread_create(&n->thread, NULL, tcp_receiver_thread, tcp_node);
                }
                break;
            }
            default:
                break; /* Stupip brek :P */
        }
        return 0;
    }
    return_close_socket_error:
        close(sockfd);
    return_error:
        return EOF;
}

void disconnect_tcp_server(struct tcp_node *node){
    if(node){
        node->run = 0;
        sleep(1);
        memset(tcp_node, 0, sizeof(struct tcp_node));
    }
}

int start_tcp_socket(void) {
    tcp_pool = (struct tcp_node*)calloc(0x400, sizeof(struct tcp_node));
    if(!tcp_pool)
        return EOF;
    tcp_pool_size = 0x400;
    return 0;
}

void stop_tcp_socket(void) {
    struct tcp_node *p = tcp_pool; *p1 = p + tcp_pool_size;
    do{
        if(p)
            p->run = 0;
    }while(++p < p1);
    sleep(1);
    if(tcp_pool)
        free(tcp_pool);
}


// TODO Listar as iinterfaces de rede

/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>

int main() {
    struct ifaddrs *ifaddr, *ifa;
    char ip[INET6_ADDRSTRLEN];

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    printf("Interfaces de rede e IPs:\n\n");

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr)
            continue;

        int family = ifa->ifa_addr->sa_family;

        if (family == AF_INET) { // IPv4
            struct sockaddr_in *sa = (struct sockaddr_in *)ifa->ifa_addr;
            inet_ntop(AF_INET, &(sa->sin_addr), ip, sizeof(ip));
            printf("Interface: %s\tIPv4: %s\n", ifa->ifa_name, ip);
        } else if (family == AF_INET6) { // IPv6 (opcional)
            struct sockaddr_in6 *sa6 = (struct sockaddr_in6 *)ifa->ifa_addr;
            inet_ntop(AF_INET6, &(sa6->sin6_addr), ip, sizeof(ip));
            printf("Interface: %s\tIPv6: %s\n", ifa->ifa_name, ip);
        }
    }

    freeifaddrs(ifaddr);
    return 0;
}

*/