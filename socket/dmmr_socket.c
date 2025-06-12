#include <dmmr_socket.h>
#include <socket_tcp.h>
#include <socket_udp.h>

static union protocol_base_cb *cap = 0;
static unsigned cap_size = 0;
static unsigned cap_count = 0;

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

static inline void dispatcher_udp(struct node *n, unsigned nl){
    if(u && nl)
        (void)udp_send_to_client(n, nl);
}

static inline void dispatcher_tcp(struct node *n, unsigned nl){
    if(u && nl)
        (void)tcp_send_to_client(n, nl);
}

static void dispatcher(protocol_base_cb *u, struct node *n, unsigned nl){
    if(node){
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

static inline int start_dispatcher_udp(ezp_addr_type, ezp_addr_type family, uint16_t port, const char* ip, void(*dispatcher_udp)(struct udp_node*)){
    ret = udp_server_is_active();
    if(!ret)
        (void)connect_udp_server();
}

static void started_receiver_tcp_cb()

static inline int start_dispatcher_tcp(ezp_addr_type, ezp_addr_type family, uint16_t port, const char* ip, void(*dispatcher_tcp)(protocol_base_cb*)){
    unsigned char buf[sizeof(struct sockaddr_in6)];
    cb = get_union_protocol_base_cb();
    cb.tcp = get_tcp_node();
    if(cb->tcp){
        ezp_addr_type tyte = dns2ipaddr(ip, buf);
        switch(type){
            case EZP_IPV4:
                __vcpy(&(cb->tcp->proto.ipv4), buf, sizeof(struct sockaddr_in));
                break;
            case AF_INET6:
                __vcpy(&(cb->tcp->proto.ipv6), buf, sizeof(struct sockaddr_in6));
                break;
            default:
                break;
        }
    cb->tcp->proto = proto_tcp_t;
    cb->tcp->port = port;
    cb->tcp->node.family = family;
    cb->tcp->on_accept_cb = this->acception_cb;
    cb->tcp->dispatcher_cb = dispatcher_cb_tcp;
    ret = connect_tcp_server(tcp_node, ip);
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
        }
            break;
        }
        case proto_none_udp:
        default:
            break;
    }
    return 0;
}

static int start_acception(proto_t proto, ezp_addr_type family, uint16_t port, const char* ip) {
    int ret;
    union protocol_base_cb *cb;

    switch (proto) {
        case proto_udp_t:{
            unsigned char buf[sizeof(struct sockaddr_in6)];
            cb = get_union_protocol_base_cb();
            cb->udp = get_udp_node();
            if(cb.udp){
                ezp_addr_type tyte = dns2ipaddr(ip, buf);
                switch(type){
                    case EZP_IPV4:
                        __vcpy(&(cb->udp->proto.ipv4), buf, sizeof(struct sockaddr_in));
                        break;
                    case AF_INET6:
                        __vcpy(&(cb->udp->proto.ipv6), buf, sizeof(struct sockaddr_in6));
                        break;
                    default:
                        break;
                }
                cb->udp->proto = proto_tcp_t;
                cb->udp->port = port;
                cb->udp->node.family = family;
                cb->udp->on_accept_cb = this->acception_cb;
                cb->udp->dispatcher_cb = dispatcher_cb_tcp;
                ret = start_udp_service(cb->udp);
                //TODO Error treatment
            }
        }
            break;
        case proto_tcp_t:{
            unsigned char buf[sizeof(struct sockaddr_in6)];
            cb = get_union_protocol_base_cb();
            cb.tcp = get_tcp_node();
            if(cb->tcp){
                ezp_addr_type tyte = dns2ipaddr(ip, buf);
                switch(type){
                    case EZP_IPV4:
                        __vcpy(&(cb->tcp->proto.ipv4), buf, sizeof(struct sockaddr_in));
                        break;
                    case AF_INET6:
                        __vcpy(&(cb->tcp->proto.ipv6), buf, sizeof(struct sockaddr_in6));
                        brea;
                    default:
                        break;
                }
                cb->tcp->proto = proto_tcp_t;
                cb->tcp->port = port;
                cb->tcp->node.family = family;
                cb->tcp->on_accept_cb = this->acception_cb;
                cb->tcp->dispatcher_cb = dispatcher_cb_tcp;
                ret = start_tcp_service(cb->tcp);
                //TODO Error treatment
            }
        }
            break;
        }
        case proto_none_udp:
        default:
            break;
    }
    return 0;
}

static void reload(struct cfg_server_server* __cfg_server){
}

struct dmmr_socket *new_dmmr_socket(struct cfg_server_server *cfg, void (*acception_cb)(struct node*), void (*acception_cb)(union protocol_base_cb*)){
    if(this)
        return 0;
	struct dmmr_socket *p = (struct dmmr_socket*)calloc(1, sizeof(struct dmmr_socket));
	if(p){
        cap = (union protocol_base_cb*)calloc(1, sizeof(union protocol_base_cb));
        if(cap){
            cap_size = 0x400;
		    p->acception_cb  = acception_cb;
		    p->dispatcher = dispatcher;
            p->start_acception = start_acception;
            this = p;
            (void)start_udp_socket();
            (void)start_tcp_socket();
            return p;
        }
	}
    if(p)
        free(p);
    return EOF;
}
