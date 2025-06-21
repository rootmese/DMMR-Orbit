#include <stdio.h>

#include <defs.h>
#include <parse_uri.h>
#include <dns_utils.h>
#include <dmmr_sleep.h>
#include <__mset.h>
#include <__vcpy.h>

#include <dmmr_socket.h>
#include <socket_tcp.h>
#include <socket_udp.h>
#include <node_recv_manager.h>

static union protocol_base_cb *cap = 0;
static uint32_t cap_size = 0;
static uint32_t cap_count = 0;

static struct node_fifo_buffer *nbuff = 0;
static uint32_t nbuff_size = 0;
static uint32_t nbuff_count = 0;

// TODO por mutex nesta seção do código
static pthread_mutex_t node_fifo_buffer_mutex;

static pthread_mutex_t protocol_base_cb_mutex;

static struct dmmr_circle_buffer* circle_buffer = 0;

static struct dmmr_socket *this = 0;

static void (*on_accept_cb_udp)(struct udp_node*) = 0;

static void (*on_accept_cb_tcp)(struct tcp_node*) = 0;

static void (*on_receive_cb)(struct node*) = 0;

static struct node_fifo_buffer *get_struct_node_buffer(void) {
    register struct node_fifo_buffer *p = nbuff;
    register struct node_fifo_buffer *p1 = p + nbuff_size;
    pthread_mutex_lock(&node_fifo_buffer_mutex);
    for (; p < p1; ++p)
        if(!(p->n.fd)){
            pthread_mutex_unlock(&node_fifo_buffer_mutex);
            return p;
        }
    if (nbuff_count >= nbuff_size) {
        nbuff_size *= 2;
        nbuff = (struct node_fifo_buffer*)realloc(nbuff, nbuff_size * sizeof(struct node_fifo_buffer));
        if (!nbuff){
            pthread_mutex_unlock(&node_fifo_buffer_mutex);
            return 0;
        }
    }
    struct node_fifo_buffer *ret =  nbuff + nbuff_count++;
    pthread_mutex_unlock(&node_fifo_buffer_mutex);
    return ret;
}

static inline void __delete_struct_node_buffer(struct node_fifo_buffer *p){
    if(p){
        pthread_mutex_unlock(&node_fifo_buffer_mutex);
        __mset(p, 0, sizeof(struct node_fifo_buffer));
        pthread_mutex_unlock(&node_fifo_buffer_mutex);
    }
} 

static void delete_struct_node_buffer(struct node_fifo_buffer *p){
    __delete_struct_node_buffer(p);
} 

static union protocol_base_cb *get_union_protocol_base_cb(void) {
    register union protocol_base_cb *p = cap;
    register union protocol_base_cb *p1 = cap + cap_size;
    pthread_mutex_lock(&protocol_base_cb_mutex);
    for (; p < p1; ++p)
        if (!(p->none.proto)){
            pthread_mutex_unlock(&protocol_base_cb_mutex);
            return p;
        }
    if(cap_count >= cap_size) {
        cap_size *= 2;
        cap = (union protocol_base_cb*)realloc(cap, cap_size * sizeof(union protocol_base_cb));
        if(!cap){
            pthread_mutex_unlock(&protocol_base_cb_mutex);
            return 0;
        }
    }
    union protocol_base_cb *ret =  cap + cap_count++;
    pthread_mutex_unlock(&protocol_base_cb_mutex);
    return ret;
}

static void delete_union_protocol_base_cb(union protocol_base_cb *u){
    if(u){
        pthread_mutex_unlock(&protocol_base_cb_mutex);
        __mset(u, 0, sizeof(union protocol_base_cb));
        pthread_mutex_unlock(&protocol_base_cb_mutex);
    }
} 

static void on_receive_connection_cb(struct node_recv_manager *nrm){
    if(nrm){
        unsigned count;
        struct node *n;
        struct node_fifo_buffer *nb = 0;
        nb = get_struct_node_buffer();
        pthread_mutex_lock(&(nrm->mutex));
        uint8_t pos = 0;
        while(n = get_buzy_node(nrm, &pos)){
            pthread_mutex_lock(&(circle_buffer->fifo_lock));
            __vcpy(&(nb->n), n, sizeof(struct node));
            TAILQ_INSERT_TAIL(&(circle_buffer->fifo), nb, tailq);
            pthread_mutex_unlock(&(circle_buffer->fifo_lock));
        };
        pthread_mutex_unlock(&(nrm->mutex));
        __delete_struct_node_buffer(nb);
    }
}

static inline void dispatcher_udp(struct node *n, unsigned nl){
    (void)udp_send_to_client(n, nl);
}

static inline void dispatcher_tcp(struct node *n, unsigned nl){
    (void)tcp_send_to_client(n, nl);
}

static void dispatcher(union protocol_base_cb *u, struct node *n, unsigned nl){
    if(n && nl){
        switch(u->none.proto){
            case proto_udp_t:
                dispatcher_udp(n, nl);
                break;
            case proto_tcp_t:
                dispatcher_tcp(n, nl);
                break;
            default:
                break; /* Stupid break */
        }
    }
}

static inline int start_dispatcher_udp(const char* ip, uint16_t port){
    int ret = udp_server_is_active();
    if(!ret)
        (void)connect_udp_server();
}

static inline int start_dispatcher_tcp(const char* ip, uint16_t port){
    unsigned char buf[sizeof(struct sockaddr_in6)];
    union protocol_base_cb *cb = get_union_protocol_base_cb();
    if(cb){
        int ret;
        ezp_addr_type type = dns2ipaddr(ip, buf);
        switch(type){
            case AF_INET:
                __vcpy(&(cb->tcp.node_cfg.ipv4), buf, sizeof(struct sockaddr_in));
                break;
            case AF_INET6:
                __vcpy(&(cb->tcp.node_cfg.ipv6), buf, sizeof(struct sockaddr_in6));
                break;
            default:
                break;
        }
        cb->tcp.proto = proto_tcp_t;
        cb->tcp.node_cfg.port = port;
        cb->tcp.node_cfg.family = type;
        cb->tcp.on_accept_cb = on_accept_cb_tcp;
        cb->tcp.on_receive_cb = on_receive_connection_cb;
        ret = connect_tcp_server(&(cb->tcp));
        return (ret) ? (EOF) : (0);
        }
    }

int create_dispatcher_from_uri(const unsigned char *uri){
    if(uri){
        uint16_t port;
        char host_buf[0x100];
        __mset(host_buf, 0, sizeof(host_buf));
        proto_t proto = parse_protocol_host_port(uri, host_buf, sizeof(host_buf), &port);
        switch (proto) {
            case proto_udp_t:
                return start_dispatcher_udp(host_buf, port);
                break;
            case proto_tcp_t:
                return start_dispatcher_tcp(host_buf, port);
                break; /* Stupid Break */
        }
    }
    return EOF;
}

static int start_acception(proto_t proto, uint16_t port, const char* ip) {
    int ret = -1;
    union protocol_base_cb *cb;

    switch (proto) {
        case proto_udp_t: {
            unsigned char buf[sizeof(struct sockaddr_in6)];
            cb = get_union_protocol_base_cb();
            if (cb) {
                ezp_addr_type type = dns2ipaddr(ip, buf);
                switch (type) {
                    case AF_INET:
                        __vcpy(&(cb->udp.node_cfg.ipv4), buf, sizeof(struct sockaddr_in));
                        break;
                    case AF_INET6:
                        __vcpy(&(cb->udp.node_cfg.ipv6), buf, sizeof(struct sockaddr_in6));
                        break;
                    default:
                        return -1; // endereço inválido
                }
                cb->udp.proto = proto_udp_t;
                cb->udp.node_cfg.port = port;
                cb->udp.node_cfg.family = type;
                cb->udp.on_accept_cb = on_accept_cb_udp;
                cb->udp.on_receive_cb = on_receive_connection_cb;
                ret = start_udp_service(&(cb->udp));
                
            }
            break;  // fim case proto_udp_t
        }

        case proto_tcp_t: {
            unsigned char buf[sizeof(struct sockaddr_in6)];
            cb = get_union_protocol_base_cb();
            if (cb) {
                ezp_addr_type type = dns2ipaddr(ip, buf);
                switch (type) {
                    case AF_INET:
                        __vcpy(&(cb->tcp.node_cfg.ipv4), buf, sizeof(struct sockaddr_in));
                        break;
                    case AF_INET6:
                        __vcpy(&(cb->tcp.node_cfg.ipv6), buf, sizeof(struct sockaddr_in6));
                        break;
                    default:
                        return -1; // endereço inválido
                }
                cb->tcp.proto = proto_tcp_t;
                cb->tcp.node_cfg.port = port;
                cb->tcp.node_cfg.family = type;
                cb->tcp.on_accept_cb = on_accept_cb_tcp;
                cb->tcp.on_receive_cb = on_receive_connection_cb;
                ret = start_tcp_service(&(cb->tcp));
            }
            break;  // fim case proto_tcp_t
        }

        default:
            return -1;  // protocolo inválido
    }  // fim switch

    return ret;
}  // fim função start_acception



static int start_accept_from_uri(const unsigned char *uri){
    if(uri){
        uint16_t port;
        char host_buf[0x100];
        __mset(host_buf, 0, sizeof(host_buf));
        proto_t proto = parse_protocol_host_port(uri, host_buf, sizeof(host_buf), &port);
        if(!(proto == proto_none_t))
            return start_acception(proto, port, host_buf);
    }
    return EOF;
}

static void reload(void){
}

static void set_acceptioncb_udp(void (*on_accept_cb)(struct udp_node*)){
    on_accept_cb_udp = on_accept_cb;
}

static void set_acceptioncb_tcp(void (*on_accept_cb)(struct tcp_node*)){
    on_accept_cb_tcp = on_accept_cb;
}

void delete_dmmr_socket(void){
    if(cap)
        free(cap);
    if(this)
        free(this);
}

struct dmmr_socket *new_dmmr_socket(struct dmmr_circle_buffer* __circle_buffer, struct cfg_server_server *__cfg){
    if(this)
        return 0;
	struct dmmr_socket *p = (struct dmmr_socket*)calloc(1, sizeof(struct dmmr_socket));
	if(p){
        cap = (union protocol_base_cb*)calloc(0x400, sizeof(union protocol_base_cb));
        if(cap){
            cap_size = 0x400;
            nbuff = (struct node_fifo_buffer*)calloc(0x400, sizeof(struct node_fifo_buffer));
            if(nbuff){
                nbuff_size = 0x400;
                p->dispatcher = dispatcher;
                p->create_dispatcher_from_uri = create_dispatcher_from_uri;
                p->start_accept_from_uri = start_accept_from_uri;
                p->reload = reload;
                p->start_acception = start_acception;
                circle_buffer = __circle_buffer;
                pthread_mutex_init(&node_fifo_buffer_mutex, 0);
                pthread_mutex_init(&protocol_base_cb_mutex, 0);
                this = p;
                (void)start_udp_socket();
                return p;
            }
        }
	}
    if(p)
        free(p);
    return 0;
}
