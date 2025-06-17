#include <dmmr_socket.h>
#include <socket_tcp.h>
#include <socket_udp.h>

static union protocol_base_cb *cap = 0;
static unsigned cap_size = 0;
static unsigned cap_count = 0;

static struct dmmr_circle_buffer* circle_buffer = 0;
static struct dmmr_socket *this = 0;

static void (*on_accept_cb_udp)(struct udp_node*) = 0;

static void (*on_accept_cb_tcp)(struct tcp_node*) = 0;

static void (*on_receive_cb)(struct node*) = 0;

union protocol_base_cb *get_union_protocol_base_cb(void) {
    register union protocol_base_cb *p = cap;
    register union protocol_base_cb *p1 = cap + cap_size;
    for (; p < p1; ++p)
        if (!(p->run))
            return p;
    if(cap_count >= cap_size) {
        cap_size *= 2;
        cap = (union protocol_base_cb*)realloc(cap, cap_size * sizeof(union protocol_base_cb));
        if(!cap)
            return 0;
    }
    return cap + cap_count++;
}

static void on_receive_conection_cb(struct node *n){
    if(n){
        // TODO verifca lock
        struct node_buffer *nb = get_struct_node_buffer();
        __vcpy(&(nb->n), n, sizeof(struct node_buffer));
        thread_mutex_lock(&(circle_buffer->fifo_lock));
        TAILQ_INSERT_TAIL(&(circle_buffer->fifo), nb, tailq);
        pthread_mutex_unlock(&circle_buffer->fifo_lock);
    }
}

static inline void dispatcher_udp(struct node *n, unsigned nl){
    if(n && nl)
        (void)udp_send_to_client(n, nl);
}

static inline void dispatcher_tcp(struct node *n, unsigned nl){
    if(n && nl)
        (void)tcp_send_to_client(n, nl);
}

static void dispatcher(union protocol_base_cb *u, struct node *n, unsigned nl){
    if(n){
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
        ezp_addr_type type = dns2ipaddr(ip, buf);
        switch(type){
            case AF_INET:
                __vcpy(&(cb->tcp.proto.ipv4), buf, sizeof(struct sockaddr_in));
                break;
            case AF_INET6:
                __vcpy(&(cb->tcp.proto.ipv6), buf, sizeof(struct sockaddr_in6));
                break;
            default:
                break;
        }
    }
    cb->tcp.proto = proto_tcp_t;
    cb->tcp.node_cfg.port = port;
    cb->tcp.node_cfg.family = type;
    cb->tcp.on_accept_cb = ((on_accept_cb_tcp) ? (on_accept_cb_tcp) : (on_acception_connection_tcp_cb));
    cb->tcp.on_receive_cb = on_receive_conection_cb;
    ret = connect_tcp_server(cb->tcp, ip);
    if(ret)
        return EOF;
    else
        return 0;
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
    int ret;
    union protocol_base_cb *cb;

    switch (proto) {
        case proto_udp_t:{
            unsigned char buf[sizeof(struct sockaddr_in6)];
            cb = get_union_protocol_base_cb();
            if(cb){
                ezp_addr_type type = dns2ipaddr(ip, buf);
                switch(type){
                    case AF_INET:
                        __vcpy(&(cb->udp.proto.ipv4), buf, sizeof(struct sockaddr_in));
                        break;
                    case AF_INET6:
                        __vcpy(&(cb->udp.proto.ipv6), buf, sizeof(struct sockaddr_in6));
                        break;
                    default:
                        break;
                }
                cb->udp.proto = proto_udp_t;
                cb->udp.node_cfg.port = port;
                cb->udp.node_cfg.family = family;
                cb->udp.on_accept_cb = ((on_accept_cb_udp) ? (on_accept_cb_udp) : (on_acception_connection_udp_cb));
                cb->udp.on_receive_cb = on_receive_conection_cb;
                ret = start_udp_service(cb->udp);
                //TODO Error treatment
            }
        }
            break;
        case proto_tcp_t:{
            unsigned char buf[sizeof(struct sockaddr_in6)];
            cb = get_union_protocol_base_cb();
            if(cb){
                ezp_addr_type type = dns2ipaddr(ip, buf);
                switch(type){
                    case AF_INET:
                        __vcpy(&(cb->tcp.proto.ipv4), buf, sizeof(struct sockaddr_in));
                        break;
                    case AF_INET6:
                        __vcpy(&(cb->tcp.proto.ipv6), buf, sizeof(struct sockaddr_in6));
                        break;
                    default:
                        break;
                }
                cb->tcp.proto = proto_tcp_t;
                cb->tcp.node_cfg.port = port;
                cb->tcp.node_cfg.family = family;
                cb->tcp.on_accept_cb = ((on_accept_cb_tcp) ? (on_accept_cb_tcp) : (on_acception_connection_tcp_cb));
                cb->tcp.on_receive_cb = on_receive_conection_cb;
                ret = start_tcp_service(cb->tcp);
                //TODO Error treatment
            }
        }
            break;
        }
    }
    return 0;
}

static int start_accept_from_uri(const unsigned char *uri){
    if(uri){
        uint16_t port;
        char host_buf[0x100];
        __mset(host_buf, 0, sizeof(host_buf));
        proto_t proto = parse_protocol_host_port(uri, host_buf, sizeof(host_buf), &port);
        if(!(proto == proto_none_t))
            return start_acception(proto, port, host);
    }
    return EOF;
}

static void reload(struct cfg_server_server* __cfg_server){
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
        fee(this);
}

struct dmmr_socket *new_dmmr_socket(struct dmmr_circle_buffer* __circle_buffer, struct cfg_server_server *__cfg){
    if(this)
        return 0;
	struct dmmr_socket *p = (struct dmmr_socket*)calloc(1, sizeof(struct dmmr_socket));
	if(p){
        cap = (union protocol_base_cb*)calloc(0x400, sizeof(union protocol_base_cb));
        if(cap){
            cap_size = 0x400;
            p->dispatcher = dispatcher;
            p->create_dispatcher_from_uri = create_dispatcher_from_uri;
            p->reload = reload;
            p->start_acception = start_acception;
            p->set_acception_cb_udp = set_acception_cb_udp;
            p->set_acception_cb_tcp = set_acception_cb_tcp;
            circle_buffer = __circle_buffer;
            this = p;
            (void)start_udp_socket();
            return p;
        }
	}
    if(p)
        free(p);
    return EOF;
}
