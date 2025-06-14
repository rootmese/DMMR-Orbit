#include <dmmr_socket.h>
#include <socket_tcp.h>
#include <socket_udp.h>

static union protocol_base_cb *cap = 0;
static unsigned cap_size = 0;
static unsigned cap_count = 0;

static struct circle_buffer* _cb = 0;
static struct dmmr_scheduler* _sched = 0;
static struct dmmr_socket *this = 0;

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
        thread_mutex_lock(&(_cb->fifo_lock));
        TAILQ_INSERT_TAIL(&(_cb->fifo), nb, tailq);
        pthread_mutex_unlock(&_cb->fifo_lock);
    }
}

static void on_acception_connection_udp_cb(struct udp_node *input){
    int ret;
    if(input){
        struct session_connection_pool *p = get_recno_slot();
        if(p){
            __vcpy(&(p->session.udp), input, sizeof(struct udp_node));
            ret = connect_udp_server(&(p->session.udp), 0);
            if(!ret){
                ret = insert_session(_circle_buffer, p->session);
                if(!ret)
                    ret = _sched->insert(_sched, p);
            }
        }
    }
    return EOF;
}

static void on_acception_connection_tcp_cb(struct tcp_node *input){
    int ret;
    if(input){
        struct session_connection_pool *p = get_recno_slot();
        if(p){
            __vcpy(&(p->session.tcp), input, sizeof(struct tcp_node));
            ret = connect_tcp_server(&(p->session.tcp), 0);
            if(!ret){
                ret = insert_session(_circle_buffer, p->session);
                if(!ret){
                    ret = _sched->insert(_sched, p);
                    return 0;
                }
        }
    }
    return EOF;
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
    cb->tcp.on_accept_cb = on_acception_connection_tcp_cb;
    cb->tcp.on_receive_cb = on_receive_conection_cb;
    ret = connect_tcp_server(cb->tcp, ip);
    if(ret)
        return EOF;
    else
        return 0;
}

static int start_dispatcher(proto_t proto, ezp_addr_type family, uint16_t port, const char *ip, void(*dispatcher)(protocol_base_cb*)){
    int ret;
    union protocol_base_cb *cb;

    switch (proto) {
        case proto_udp_t:
            break;
        case proto_tcp_t:
            break;
        }
    }
    return 0;
}

int create_dispatcher_from_uri(const unsigned char *uri){
    if(uri){
        uint16_t port;
        char host_buf[0x100];
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
                cb->udp.on_accept_cb = on_acception_connection_udp_cb;
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
                cb->tcp.on_accept_cb = on_acception_connection_tcp_cb;
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

static void reload(struct cfg_server_server* __cfg_server){
}

struct dmmr_socket *new_dmmr_socket(struct circle_buffer* cb, struct dmmr_scheduler* sched, struct cfg_server_server *cfg, void (*acception_cb)(struct node*)){
    if(this)
        return 0;
	struct dmmr_socket *p = (struct dmmr_socket*)calloc(1, sizeof(struct dmmr_socket));
	if(p){
        cap = (union protocol_base_cb*)calloc(0x400, sizeof(union protocol_base_cb));
        if(cap){
            cap_size = 0x400;
	        p->acception_cb  = acception_cb;
            p->dispatcher = dispatcher;
            p->start_acception = start_acception;
            this = p;
            (void)start_udp_socket();
            return p;
        }
	}
    if(p)
        free(p);
    return EOF;
}
